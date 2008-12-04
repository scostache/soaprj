/* 
 db_backend.cpp - database access methods
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

/* C++ headers */
#include <sstream>
#include <cstring>
#include <vector>

/* C headers */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* my headers */
#include "db_backend.hpp"
#include "hybfs.h"
#include "hybfsdef.h"
#include "misc.h"

using namespace std;

/* wrapper for the error condition related to the database */
#define DB_ERROR(cond, message, db) \
	if(cond) { \
		fprintf(stderr, "%s: %s\n", message, sqlite3_errmsg(db)); \
		return -1; \
	}

/* wrapper for the error messages that we get from the data base */
#define DB_PRINTERR(message,db) \
	fprintf(stderr, "%s: %s\n",message, sqlite3_errmsg(db));

DbBackend::DbBackend(const char *path)
{
	db_path.assign(path);
	db = NULL;

	/* init virtual query caches */
}

DbBackend::~DbBackend()
{
	db_close_storage();

	/* destroy the caches here */

}

void DbBackend::db_close_storage()
{
	int ret;
	
	DBG_SHOWFC();

	/* close all databases here */
	if (!db)
		return;

	ret = sqlite3_close(db);
	if(ret != SQLITE_OK)
		DB_PRINTERR("Error at closing the database: ",db);
	db = NULL;
	
}

int DbBackend::run_simple_query(const char* query)
{
	int ret;
	sqlite3_stmt *select= NULL;

	ret = sqlite3_prepare_v2(db, query, -1, &select, 0);
	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing query: ",db);
		goto out;
	}
	ret = sqlite3_step(select);
	if (ret != SQLITE_DONE) {
		DB_PRINTERR("Running query: ",db);
		goto out;
	}

	ret = 0;

out: 
	if (select)
		ret = sqlite3_finalize(select);
	
	return ret;
}

int DbBackend::create_main_tables()
{
	int ret, empty, exit;
	sqlite3_stmt *select= NULL;

	/* check if we already have the tables */
	ret = sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master "
		"WHERE tbl_name LIKE 'tags' AND type  == 'table'; ", -1,
	                &select, 0);
	if (ret!=SQLITE_OK || !select) {
		DB_PRINTERR("Preparing select: ",db);
		sqlite3_finalize(select);
		return -1;
	}
	empty = 0;
	exit = 0;
	ret = sqlite3_step(select);
	switch(ret) {
	case SQLITE_DONE:
		empty = 1;
		exit = 0;
		break;
	case SQLITE_ROW:
		empty = 0;
		exit = 0;
		break;
	default:
		DB_PRINTERR("Database error: ",db);
		exit = 1;
		break;
	}
	/* need to exit due to an error? */
	if (exit) {
		sqlite3_finalize(select);
		return -1;
	}
	ret  = sqlite3_finalize(select);
	if(ret != SQLITE_OK) {
		DB_PRINTERR("Database error: ",db);
		return -1;
	}
	/* we have an empy database, create the tables */
	if (empty) {
		DBG_PRINT("HYBFS: Creating main table \n");

		ret = run_simple_query("CREATE TABLE tags("
			"tag_id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
			"tag VARCHAR(256), \n"
			"value VARCHAR(256),"
			"UNIQUE (tag, value));");
		DB_ERROR(ret != SQLITE_OK,"Table TAGS ", db);

		ret = run_simple_query("CREATE TABLE files("
			"ino INTEGER PRIMARY KEY, \n"
			"mode INTEGER, \n"
			"path VARCHAR(256)) ;");
		DB_ERROR(ret != SQLITE_OK,"Table FILES ", db);

		ret = run_simple_query("CREATE TABLE assoc("
			"ino INTEGER, \n"
			"tag_id INTEGER, \n"
			"PRIMARY KEY (ino, tag_id) \n );");
		DB_ERROR(ret != SQLITE_OK,"Table ASSOC ", db);

	}

	return 0;
}

int DbBackend::db_init_storage()
{
	int ret = 0;

	DBG_SHOWFC();

	/* open the database for the given branch */
	DBG_PRINT("env path is: %s \n", db_path.c_str());

	ret = sqlite3_open(db_path.c_str(), &db);
	if (ret) {
		DB_PRINTERR("Can't open database", db);
		sqlite3_close(db);
		return ret;
	}
	
	ret = create_main_tables();
	if (ret) {
		db_close_storage();
		return -1;
	}
	
	return 0;
}

int DbBackend::db_add_tag(const char *tag, const char *value)
{
	int ret = -1;
	sqlite3_stmt *select= NULL;

	DBG_SHOWFC();

	sqlite3_exec(db, "BEGIN", 0, 0, 0);
	/* adds the info in all the tables */
	ret = sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO tags (tag, value) VALUES (?1, ?2);",
	                -1, &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	sqlite3_bind_text(select, 1, tag, -1, SQLITE_STATIC);
	if(value != NULL)
		sqlite3_bind_text(select, 2, value, -1, SQLITE_STATIC);
	else
		sqlite3_bind_text(select, 2, NULL_VALUE, -1, SQLITE_STATIC);
	
	ret = sqlite3_step(select);
	if(ret != SQLITE_DONE) {
		DB_PRINTERR("Executing insert: ",db);
		goto error;
	}
	
	ret = sqlite3_finalize(select);
	select = NULL;
	
	/* get the tag number, so we can use it for the other insertions */
	if(ret != SQLITE_OK){
		DB_PRINTERR("Error at processing tag insert: ",db);
		goto error;
	}
	sqlite3_exec(db, "COMMIT", 0, 0, 0);
	
	return db_check_tag(tag, value);
	
error:
	if(select)
		sqlite3_finalize(select);
	sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
	
	return -1;
}

int DbBackend::db_add_file(file_info_t * finfo)
{
	int ret;
	sqlite3_stmt *select= NULL;
	
	DBG_SHOWFC();
	
	/* adds the info in all the tables */
	ret = sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO files VALUES (?1, ?2, ?3);",
	                -1, &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	sqlite3_bind_int64(select, 1, finfo->fid);
	sqlite3_bind_int(select, 2, finfo->mode);
	sqlite3_bind_text(select, 3, &finfo->name[0], finfo->namelen, SQLITE_STATIC);
	sqlite3_step(select);

	ret = sqlite3_finalize(select);
	select = NULL;

	/* get the tag number, so we can use it for the other insertions */
	if (ret != SQLITE_OK) {
		DB_PRINTERR("Error at processing tag insert: ",db);
		goto error;
	}
	
error:
	if(select)
		sqlite3_finalize(select);
	return -1;
}

int DbBackend::db_add_file_info(vector<string> *tags, file_info_t * finfo)
{
	int ret = 0;
	int tag_id;
	size_t fpos;
	sqlite3_stmt *select= NULL;
	string tag_value;
	string tag, value;
	
	DBG_SHOWFC();

	/* now adds the file info */
	ret = db_add_file(finfo);
	/* TODO should I delete the tag from the DB? */
	/*if (ret) {
	 	commits = "ROLLBACK";
		goto error;
	}*/

	/* now the association */
	ret = sqlite3_prepare_v2(db, "INSERT INTO assoc VALUES (?1, ?2);", -1,
	                &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	for (vector<string>::iterator tok_iter = tags->begin(); tok_iter
	                != tags->end(); ++tok_iter) {

		/* break the tag in (tag-value) */
		fpos = (*tok_iter).find(":");
		if(fpos != string::npos) {
			tag = (*tok_iter).substr(0, fpos);
			value = (*tok_iter).substr(fpos+1, (*tok_iter).length() - 1);
			tag_id = db_add_tag(tag.c_str(), value.c_str());
		}
		else
			tag_id = db_add_tag((*tok_iter).c_str(), NULL);

		/* adds the tag info */
		if (tag_id <= 0) {
			continue;
		}

		sqlite3_bind_int(select, 1, finfo->fid);
		sqlite3_bind_int(select, 2, tag_id);
		ret = sqlite3_step(select);
		if (ret != SQLITE_DONE) {
			DB_PRINTERR("Error at trying to insert a tag: ",db);
			goto error;
		}
		sqlite3_reset(select);
	}
	
	sqlite3_finalize(select);
	ret = 0;
	select = NULL;

error: 
	if (select)
		sqlite3_finalize(select);
	
	return ret;
}

int DbBackend::db_delete_file_tag(const char *tag, const char *path)
{
	int ret = 0;

	/* TODO */
	
	return ret;
}

int DbBackend::db_delete_file_info(const char *abspath)
{
	int ret = -1;
	string sql;
	
	DBG_SHOWFC();

	if(abspath == NULL)
		return -1;
	
	sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
	/* delete the association info from the table */
	sql = "DELETE FROM assoc "
			"WHERE assoc.ino IN (SELECT assoc.ino FROM assoc, files "
			"WHERE files.path LIKE '";
	sql.append(abspath);
	sql.append("' AND assoc.ino = files.ino );");
	
	ret = run_simple_query(sql.c_str());
	if(ret) {
		PRINT_ERROR("Error deleting file associations\n");
		goto error;
	}
	/* delete the file info */
	sql.assign("DELETE FROM files WHERE files.path LIKE '");
	sql.append(abspath);
	sql.append("';");
	ret = run_simple_query(sql.c_str());
	if(ret) {
		PRINT_ERROR("Error deleting file info\n");
		goto error;
	}
	/* if a tag has no associated files, then delete the tag */
	ret = run_simple_query("DELETE FROM tags WHERE "
			"(tags.tag_id IN (SELECT tags.tag_id FROM tags "
			"EXCEPT SELECT assoc.tag_id FROM assoc));");
	
error:
	if(ret)
		sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	else
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
	
	return ret;
}

int DbBackend::db_delete_allfile_info(const char *tag, const char *value)
{
	int ret = -1;
	string sql;

	DBG_SHOWFC();

	if(tag == NULL)
		return -1;
	
	sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
	/* delete the associations info from the table for this tag*/
	sql = "DELETE FROM assoc WHERE tags.tag = '";
	sql.append(tag);
	if(value) {
		sql.append("' AND tags.value LIKE '");
		sql.append(value);
	}
	sql.append("' AND tags.tag_id = assoc.ino ;");

	ret = run_simple_query(sql.c_str());
	if (ret)
		goto error;

	/* delete the tag , or the tag-value*/
	sql.assign("DELETE FROM tags WHERE tags.tag LIKE '");
	sql.append(tag);
	if(value) {
		sql.append("' AND tags.value LIKE '");
		sql.append(value);
	}
	sql.append("';");
	ret = run_simple_query(sql.c_str());
	if (ret)
		goto error;

	/* delete the file info - only if there are no more associations for
	 * this file */
	ret = run_simple_query("DELETE FROM files WHERE "
		"(files.ino IN (SELECT files.ino FROM files "
		"EXCEPT SELECT assoc.ino FROM assoc));");

error: 
	if (ret)
		sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
	else
		sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

	
	return ret;
}

int DbBackend::db_check_tag(const char *tag, const char *value)
{
	int res = 0;
	int tag_id;
	sqlite3_stmt *select= NULL;

	DBG_SHOWFC();

	DBG_PRINT("I check tag %s : %s \n", tag, value);
	/* search for the tag id in the table */
	
	res = sqlite3_prepare_v2(db, "SELECT tag_id FROM tags WHERE  "
			"tag == ?1 AND value == ?2 ;", -1, &select, 0);

	if (res != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}

	sqlite3_bind_text(select, 1, tag, -1, SQLITE_STATIC);
	if (value == NULL)
		sqlite3_bind_text(select, 2, NULL_VALUE, -1, SQLITE_STATIC);
	else
		sqlite3_bind_text(select, 2, value, -1, SQLITE_STATIC);

	res = sqlite3_step(select);
	switch (res)
	{
	case SQLITE_DONE:
		tag_id = 0;
		break;
	case SQLITE_ROW:
		tag_id = sqlite3_column_int(select, 0);
		break;
	default:
		tag_id = -1;
		DB_PRINTERR("Getting tag id: ",db)
		;
	}
	if (tag_id < 0)
		goto error;

	res = sqlite3_finalize(select);
	if (res != SQLITE_OK)
		DB_PRINTERR("Error at trying to insert a tag: ",db);

	DBG_PRINT("Getting tag id: %d \n",tag_id);

	return (res != SQLITE_OK) ? -1 : tag_id;

error: 
	if (select)
		sqlite3_finalize(select);
	
	return -1;
}


static int tags_callback(void *data, int argc, char **argv, char **colname)
{
	ostringstream component;
	list<string> *tags = (list<string> *) data;
	
	if(argc <1)
		return 0;
	
	if(argv[0] == NULL)
		return 0;
	
	if(strlen(argv[0]) == 0)
		return 0;
	
	component << '(' << argv[0] << ')';
	tags->push_back(component.str());
	
	return 0;
}

list<string> * DbBackend::db_get_tags(const char * path)
{
	int res = 0;
	char * err;
	const char * lpath;
	ostringstream sql_string;
	list<string> *tags = new list<string>;
	
	if(tags == NULL)
		return NULL;

	DBG_SHOWFC();
	DBG_PRINT("my path is #%s#\n", path);
	sql_string <<  "SELECT DISTINCT tag FROM tags";
	if(path && path[0] != '\0') {
		lpath = path;
		if(lpath[0] == '/')
			lpath++;
		sql_string << ", files, assoc WHERE files.path LIKE '" << lpath;
		sql_string << "%' AND tags.tag_id = assoc.tag_id "
				"AND files.ino = assoc.ino";
	
	}
	sql_string << ";";
	DBG_PRINT("my final query is : %s \n", sql_string.str().c_str());
	
	res = sqlite3_exec(db, sql_string.str().c_str(), tags_callback, tags, &err);

	if( res !=SQLITE_OK ){
	    PRINT_ERROR("SQL error: %s\n", err);
	    sqlite3_free(err);
	    
	    tags->clear();
	    delete tags;
	    
	    return NULL;
	  }
	
	return tags;
}

static int tags_values_callback(void *data, int argc, char **argv, char **colname)
{
	ostringstream component;
	list<string> *tags = (list<string> *) data;
	
	assert(tags);
	assert(argv);
	assert(colname);
	
	if(argc <2)
		return 0;
	
	if(argv[0] == NULL || argv[1] == NULL)
		return 0;
	
	if(strlen(argv[0]) == 0 || strlen(argv[1]) == 0 || strcmp(argv[1], NULL_VALUE) == 0)
		return 0;
	
	component << '(' << argv[0] << ':' << argv[1] << ')';
	
	tags->push_back(component.str());
	
	return 0;
}

list<string> * DbBackend::db_get_tags_values(const char *path)
{
	int res = 0;
	char * err;
	const char *lpath;
	ostringstream sql_string;
	list<string> *tags = new list<string>;
	
	if (tags == NULL)
		return NULL;

	DBG_SHOWFC();
	sql_string <<  "SELECT DISTINCT tag, value FROM tags";
	if(path && path[0] != '\0') {
		lpath = path;
		if(lpath[0] == '/')
			lpath++;
		sql_string << ", files, assoc WHERE files.path LIKE '" << lpath;
		sql_string << "%' AND tags.tag_id = assoc.tag_id "
				"AND files.ino = assoc.ino";
	
	}
	sql_string << ";";
	
	res = sqlite3_exec(db,sql_string.str().c_str(), tags_values_callback,
	                tags, &err);

	if (res !=SQLITE_OK) {
		PRINT_ERROR("SQL error: %s\n", err);
		sqlite3_free(err);

		tags->clear();
		delete tags;

		return NULL;
	}

	return tags;
}

int DbBackend::db_get_filesinfo(list<string> *tags, string *path,
                                void * buf, filler_t filler)
{
	/* get all files only for a tag:value pair, possibly with the specified path..*/
	/* TODO: do it for multiple tags */
	string tag, value;
	const char * pathchr = NULL;
	
	if(path)
		pathchr = path->c_str();
	
	break_tag(&(tags->front()), &tag, &value);
	DBG_PRINT("I have tag #%s# value #%s# \n", tag.c_str(), value.c_str());
	
	return db_get_files(pathchr, tag.c_str(), value.c_str(), buf, filler);
}

int DbBackend::db_get_files(const char * path, const char * tag, 
                            const char *value, void * buf, filler_t filler)
{
	sqlite3_stmt* sql;
	int res, fill;
	ostringstream sql_string;
	list <string> *filepaths = new list <string>();
	
	/* build the query */
	sql_string << "SELECT files.ino, mode, path FROM files, tags, assoc WHERE ";
	if(path) {
		if(path[0] != '\0') {
			const char * pathl = (path[0] == '/') ? (path+1) : path;
			sql_string<< " path LIKE '" << pathl << "%' AND ";
		}
	}
	sql_string << "tags.tag = ?1 AND ";
	if(value[0]!='\0')
		sql_string << "tags.value = ?2 AND ";
	sql_string << "tags.tag_id = assoc.tag_id AND files.ino = assoc.ino ;";
	
	/* done building the query */

	res = sqlite3_prepare_v2(db, sql_string.str().c_str(), -1, &sql, 0);
	if (res != SQLITE_OK || !sql) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}

	sqlite3_bind_text(sql, 1, tag, -1, SQLITE_STATIC);
	if (value[0] != '\0') {
		sqlite3_bind_text(sql, 2, value, -1, SQLITE_STATIC);
		DBG_PRINT("value for tag is not zero!\n");
	}
	
	while ((res = sqlite3_step(sql)) == SQLITE_ROW) {
		stat_t st;
		char *relpath;
		
		memset(&st, 0, sizeof(stat_t));
		st.st_ino = sqlite3_column_int64(sql, 0);
		st.st_mode = sqlite3_column_int(sql, 1);
		char *abspath = (char *)sqlite3_column_text(sql, 2);
		if(abspath == NULL) {
			res = -1;
			break;
		}
		/* remember the path */
		filepaths->push_back(string(abspath));
		/* strip from abspath the path already given, if any */
		relpath = abspath;
		if(path) {
			if(path[0] != '\0')
				relpath = abspath + strlen(path);
		}
		
		fill = filler(buf, relpath, &st, 0);
		if(fill) {
			res = SQLITE_DONE;
			break;
		}
	}

	if (res != SQLITE_DONE) {
		DB_PRINTERR("Error at processing select: ",db);
		goto error;
	}
	res = sqlite3_finalize(sql);
	sql = NULL;
	if (res != SQLITE_OK) {
		DB_PRINTERR("Error at finalizing select: ",db);
		goto error;
	}
	/*TODO: get the tags and tag-value pairs for the files*/
	
	filepaths->clear();
	delete filepaths;
	
	res = 0;

error: 
	if (sql)
		sqlite3_finalize(sql);
	
	return res;
}
