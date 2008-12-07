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
		ret = -1; \
		goto out; \
	}

/* wrapper for the error messages that we get from the data base */
#define DB_PRINTERR(message,db) \
	fprintf(stderr, "%s: %s\n",message, sqlite3_errmsg(db));

DbBackend::DbBackend(const char *_path, const char *_vdir_path)
{
	db_path.assign(_path);
	db = NULL;

	vdir_path = _vdir_path;
	
	DBG_PRINT("DbBackend Constructor: I have directories: %s %s\n",
	          _path, _vdir_path);
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
	sqlite3_stmt *select;

	ret = sqlite3_prepare_v2(db, query, -1, &select, NULL);
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
		if(ret != SQLITE_OK)
			DB_PRINTERR("Finalize query: ",db);
	return ret;
}

int DbBackend::create_main_tables()
{
	int ret, empty, exit;
	int do_trans;
	sqlite3_stmt *select= NULL;

	do_trans = 0;
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
	/* we have an empty database, create the tables */
	if (empty) {
		DBG_PRINT("HYBFS: Creating main tables \n");
		
		sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
		do_trans = 1;
		
		ret = run_simple_query("CREATE TABLE tags("
			"tag_id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
			"tag VARCHAR(256), \n"
			"value VARCHAR(256),"
			"UNIQUE (tag, value));");
		DB_ERROR(ret != SQLITE_OK,"Table TAGS ", db);

		ret = run_simple_query("CREATE TABLE files("
			"ino INTEGER PRIMARY KEY, \n"
			"mode INTEGER, \n"
			"path VARCHAR(256), \n"
			"tags VARCHAR(512)) ;");
		DB_ERROR(ret != SQLITE_OK,"Table FILES ", db);

		ret = run_simple_query("CREATE TABLE assoc("
			"ino INTEGER, \n"
			"tag_id INTEGER, \n"
			"PRIMARY KEY (ino, tag_id) \n );");
		DB_ERROR(ret != SQLITE_OK,"Table ASSOC ", db);

		/* add a trigger for delete from assoc */
		ret = run_simple_query("CREATE TRIGGER delete_trig AFTER "
				"DELETE ON assoc \n"
				"BEGIN \n"
				"DELETE FROM files WHERE files.tags = '' ; \n"
				"DELETE FROM tags WHERE tags.tag_id IN \n"
				"(SELECT tags.tag_id FROM tags EXCEPT \n"
				"SELECT assoc.tag_id FROM assoc ); \n "
				"END ;");
		DB_ERROR(ret != SQLITE_OK,"Trigger error ", db);
		
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
	}
	ret = 0;
	
out:
	if(do_trans) {
		if(ret)
			sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
		else
			sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
	}
	
	return ret;
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

	/* adds the info in the tag table */
	ret = sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO tags (tag, value) "
			"VALUES (?1, ?2);", -1, &select, 0);

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

	return db_check_tag(tag, value);
	
error:
	if(select)
		sqlite3_finalize(select);

	return -1;
}

int DbBackend::db_add_file(file_info_t * finfo)
{
	int ret;
	sqlite3_stmt *select= NULL;
	
	DBG_SHOWFC();
	
	/* adds the info in the file table */
	ret = sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO files (ino,mode,path,tags)"
			" VALUES (?1, ?2, ?3,' ');", -1, &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	sqlite3_bind_int64(select, 1, finfo->fid);
	sqlite3_bind_int(select, 2, finfo->mode);
	sqlite3_bind_text(select, 3, finfo->name, finfo->namelen, SQLITE_TRANSIENT);
	sqlite3_step(select);

	ret = sqlite3_finalize(select);
	select = NULL;

	if (ret != SQLITE_OK) {
		DB_PRINTERR("Error at processing file insert: ",db);
		goto error;
	}
	ret = 0;
	
error:
	if(select)
		sqlite3_finalize(select);
	return ret;
}


int DbBackend::db_add_tag_info(vector<string> *tags, file_info_t * finfo, int behaviour)
{
	int ret;
	int tag_id;
	size_t fpos;
	sqlite3_stmt *select;
	const char *sql = NULL;
	string tag_value;
	string tag, value;
	string file_tagsp;
	ostringstream file_tags;
	
	/* now the association */
	ret = sqlite3_prepare_v2(db, "INSERT INTO assoc VALUES (?1, ?2);", -1,
	                &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	for (vector<string>::iterator tok_iter = tags->begin(); tok_iter
	                != tags->end(); ++tok_iter) {
		DBG_PRINT("I have tag %s\n", (*tok_iter).c_str());
		/* break the tag in (tag-value) */
		fpos = (*tok_iter).find(":");
		if (fpos != string::npos) {
			tag = (*tok_iter).substr(0, fpos);
			value = (*tok_iter).substr(fpos+1, (*tok_iter).length() - 1);
			tag_id = db_add_tag(tag.c_str(), value.c_str());
		}
		else {
			tag_id = db_add_tag((*tok_iter).c_str(), NULL);
		}
		/* adds the tag info */
		if (tag_id <= 0) {
			PRINT_ERROR("Failed to add tag %s:%s to file %s\n",
					tag.c_str(), value.c_str(), finfo->name);
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
		
		if (fpos != string::npos)
			file_tags << tag << ":" << value;
		else
			file_tags << (*tok_iter).c_str() << ":" << NULL_VALUE;
		file_tags << " ";
	}

	ret = sqlite3_finalize(select);
	select = NULL;
	
	/* add or replace the tags to the file field */
	switch(behaviour) {
	case TAG_ADD:
		sql = "UPDATE files SET tags = tags||?2 WHERE files.path LIKE ?1 ;";
		break;
	case TAG_REPLACE:
		sql = "UPDATE files SET tags = ' '||?2 WHERE files.path LIKE ?1;";
		break;
	default:
		sql = NULL;
	}
	
	if(sql == NULL) {
		PRINT_ERROR("undefined behaviour in %s:%d",__func__,__LINE__);
		ret = -1;
		goto error;
	}
		
	ret = sqlite3_prepare_v2(db, sql, -1, &select, 0);
	if (ret != SQLITE_OK || !select) {
			DB_PRINTERR("Preparing insert: ",db);
			goto error;
	}
	
	DBG_PRINT("file path is %s with tags #%s#\n", finfo->name,file_tags.str().c_str() );
	
	ret = sqlite3_bind_text(select, 1, &finfo->name[0], finfo->namelen, SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}
	file_tagsp = file_tags.str();
	ret = sqlite3_bind_text(select, 2, file_tagsp.c_str(), file_tagsp.length(), SQLITE_STATIC);
	if(ret != SQLITE_OK) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	ret = sqlite3_step(select);
	if (ret != SQLITE_DONE) {
		DB_PRINTERR("Error at trying to insert a tag: ",db);
		goto error;
	}
	ret = 0;
	
error:
	if(select)
		sqlite3_finalize(select);
	
	return ret;	
}


int DbBackend::db_add_file_info(vector<string> *tags, file_info_t * finfo)
{
	int ret = 0;
	string tag_value;
	string tag, value;
	
	DBG_SHOWFC();

	sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
	/* now add the file info */
	ret = db_add_file(finfo);

	ret = db_add_tag_info(tags, finfo, TAG_ADD);
	if(ret != SQLITE_OK) {
		ret = -1;
		goto error;
	}
	ret = 0;

error: 
	if(ret)
		sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	else
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
		
	return ret;
}

int DbBackend::db_delete_file_tag(const char *tag, const char *value,
                                  const char *path)
{
	int ret = -1;
	string tag_value;
	string sql;
	sqlite3_stmt *select;
	
	DBG_SHOWFC();

	if(path == NULL)
		return -1;

	/* delete the association info from the table */
	sql = "DELETE FROM assoc WHERE "
	      "assoc.ino IN (SELECT assoc.ino FROM assoc, tags, files "
			"WHERE assoc.ino = files.ino AND assoc.tag_id = tags.tag_id "
			"AND files.path LIKE '";
	sql.append(path);
	sql.append("' AND  tags.tag LIKE '");
	sql.append(tag);
	sql.append("' AND value LIKE '");
	if(value != NULL) {
		sql.append(value);
	} else
		sql.append(NULL_VALUE);
	sql.append("' );");
	
	DBG_PRINT("I have the delete query :  %s\n", sql.c_str());
	ret = run_simple_query(sql.c_str());
	if(ret) {
		PRINT_ERROR("Error deleting file associations\n");
		return ret;
	}
	/* delete the tag from the string of tags */
	ret = sqlite3_prepare_v2(db,"UPDATE files SET tags = "
			"replace(files.tags, ?1, ' ') WHERE files.path LIKE ?2",
			-1, &select, 0);
		
	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing delete: ",db);
		goto error;
	}
	
	tag_value.append(" ");
	tag_value.append(tag);
	if(value != NULL) {
		tag_value.append(":");
		tag_value.append(value);
		tag_value.append(" ");
	}
	else {
		tag_value.append(":");
		tag_value.append(NULL_VALUE);
		tag_value.append(" ");
	}
	sqlite3_bind_text(select, 1, tag_value.c_str(),tag_value.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 2, path, strlen(path), SQLITE_STATIC);
	ret = sqlite3_step(select);
	if (ret != SQLITE_DONE) {
		DB_PRINTERR("Error at trying to delete a tag: ",db);
		goto error;
	}
	ret = 0;
	
error:
	if(select)
		sqlite3_finalize(select);
	if(ret)
		ret = -1;
	
	return ret;
}

int DbBackend::db_delete_file_tags(vector<string> *tags, file_info_t *finfo)
{
	int ret = 0;
	string tag;
	string value;
	size_t fpos;
	const char *tagc, *valuec;
	
	sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
	
	/* break the tags in tag:value pairs and call delete_file_tag */
	for (vector<string>::iterator tok_iter = tags->begin(); tok_iter
	                != tags->end(); ++tok_iter) {
		tagc = NULL;
		valuec = NULL;
		/* break the tag in (tag-value) */
		fpos = (*tok_iter).find(":");
		if (fpos != string::npos) {
			tag = (*tok_iter).substr(0, fpos);
			value = (*tok_iter).substr(fpos+1, (*tok_iter).length() - 1);
		}
		
		if(value.length() != 0) {
			tagc = tag.c_str();
			valuec = value.c_str();
		}
		else
			tagc = (*tok_iter).c_str();
		
		DBG_PRINT("I try to delete %s:%s for %s\n", tagc, valuec, finfo->name);
		ret = db_delete_file_tag(tagc, valuec, finfo->name);
		if(ret)
			break;
	}
	
	if(ret)
		sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	else
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);	
	
	return ret;
}

int DbBackend::db_update_file_tags(vector<string> *new_tags, file_info_t *finfo)
{
	int ret;
	string sql;
	
	sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
	/* check if the file exists */
	ret = db_check_file(&finfo->name[0]);
	if(ret == 0) {
		PRINT_ERROR("file info doesn't exist in the DB . "
				"Adding file info\n");
		ret = db_add_file(finfo);
		
		if(ret)
			return -1;
	} else {
		/* delete the associations info from the table */
		sql = "DELETE FROM assoc "
				"WHERE assoc.ino IN (SELECT assoc.ino "
				"FROM assoc, files "
				"WHERE files.path LIKE '";
		sql.append(&finfo->name[0]);
		sql.append("' AND assoc.ino = files.ino );");
		
		ret = run_simple_query(sql.c_str());
		if(ret) {
			PRINT_ERROR("Error deleting file associations\n");
			goto error;
		}
	}
	/* add the new associations */
	ret = db_add_tag_info(new_tags, finfo, TAG_REPLACE);
	if(ret != SQLITE_OK) {
		ret = -1;
		goto error;
	}
	ret = 0;
	
error:
	if(ret)
		sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	else
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
		
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
	
error:
	if(ret)
		sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	else
		sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
	
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
		DB_PRINTERR("Getting tag id: ",db);
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

int DbBackend::db_check_file(const char *path)
{
	int res = 0;
	int sqlres = 0;
	sqlite3_stmt *select = NULL;

	DBG_SHOWFC();

	DBG_PRINT("I check file %s \n", path);
	/* search for the tag id in the table */
	
	res = sqlite3_prepare_v2(db, "SELECT path FROM files WHERE  "
			"path LIKE ?1 ;", -1, &select, 0);

	if (res != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing checking file existence: ",db);
		goto error;
	}

	sqlite3_bind_text(select, 1, path, -1, SQLITE_STATIC);

	sqlres = sqlite3_step(select);
	
	if(sqlres == SQLITE_ROW)
		sqlres = 1;
	else
		sqlres = 0;
	
	res = sqlite3_finalize(select);
	if (res != SQLITE_OK)
		DB_PRINTERR("Error at trying to insert a tag: ",db);

	
	return ((res != SQLITE_OK) ? 0 : sqlres);

error: 
	if (select)
		sqlite3_finalize(select);
	
	return 0;
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
	
	if(strlen(argv[0]) == 0 || strlen(argv[1]) == 0 || 
			strcmp(argv[1], NULL_VALUE) == 0)
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


int DbBackend::db_get_files(const char * path, const char * tag, 
                            const char *value, void * buf, filler_t filler)
{
	sqlite3_stmt* sql;
	int res, fill;
	ostringstream sql_string;
	string absolute;
	
	/* build the query */
	sql_string << "SELECT path FROM files, tags, assoc WHERE ";
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
		
		char *abspath = (char *)sqlite3_column_text(sql, 2);
		if(abspath == NULL) {
			res = -1;
			break;
		}

		/* strip from abspath the path already given, if any */
		relpath = abspath;
		if(path) {
			if(path[0] != '\0')
				relpath = abspath + strlen(path);
		}
		/* build the absolute path for stat */
		absolute.assign(vdir_path);
		absolute.append(abspath);
		
		res = get_stat(absolute.c_str(), &st);
		if(res)
			break;
		
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
	
	res = 0;

error: 
	if (sql)
		sqlite3_finalize(sql);
	
	return res;
}

int DbBackend::db_get_filesinfo(string *query, string *path,
                                void * buf, filler_t filler)
{
	sqlite3_stmt* sql;
	int res, fill;
	string sqlp;
	string absolute;
	ostringstream sql_string;

	/* build the query */
	if(path) {
		if(path->length() != 0) {
			const char * pathl = path->c_str();
			if(pathl[0] == '/')
				pathl++;
			sql_string << "SELECT ino, mode, path FROM files WHERE ";
			sql_string<< " path LIKE '" << pathl << "%' INTERSECT ";
		}
	}
	sql_string << *query;
	/* done building the query */
	sqlp = sql_string.str();
	res = sqlite3_prepare_v2(db, sqlp.c_str(), sqlp.length(), &sql, 0);
	if (res != SQLITE_OK || !sql) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}

	while ((res = sqlite3_step(sql)) == SQLITE_ROW) {
		char *relpath;
		stat_t st;
		
		char *abspath = (char *)sqlite3_column_text(sql, 2);
		if(abspath == NULL) {
			res = -1;
			break;
		}
		/* strip from abspath the path already given, if any */
		relpath = abspath;
		if(path) {
			if(path->at(0) != '\0')
				relpath = abspath + path->length();
		}
		
		absolute.assign(vdir_path);
		absolute.append(abspath);
		res = get_stat(absolute.c_str(), &st);
		if(res)
			break;
		
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

	res = 0;

error: 
	if (sql)
		sqlite3_finalize(sql);
	
	return res;
}


int DbBackend::update_file_path(const char *from, const char *to)
{
	int res;
	
	sqlite3_stmt* sql;
	
	DBG_PRINT("Rename file path in DB: from=%s to=%s\n", from, to);
	res = sqlite3_prepare_v2(db, "UPDATE files SET path = ?1 "
			"WHERE files.path LIKE ?2", -1, &sql, 0);
	if (res != SQLITE_OK || !sql) {
		DB_PRINTERR("Preparing path update: ",db);
		goto error;
	}
	
	sqlite3_bind_text(sql, 1, to, -1, SQLITE_STATIC);
	sqlite3_bind_text(sql, 2, from, -1, SQLITE_STATIC);
	
	res = sqlite3_step(sql);
	if (res != SQLITE_DONE) {
		DB_PRINTERR("Error at trying to replace file path: ",db);
		goto error;
	}
	res = 0;
	
error:
	if(sql)
		sqlite3_finalize(sql);
	if(res)
		res = -1;
	
	return res;
}

int DbBackend::db_begin_transaction()
{
	int res;
	
	res = sqlite3_exec(db, "BEGIN",NULL,NULL,NULL);
	if(res != SQLITE_OK) {
		DB_PRINTERR("begin transaction error: ",db);
		return -1;
	}
	
	return 0;
}
	
int DbBackend::db_rollback()
{
	int res;
		
	res = sqlite3_exec(db, "ROLLBACK",NULL,NULL,NULL);
	if(res != SQLITE_OK) {
		DB_PRINTERR("rollback transaction error: ",db);
		return -1;
	}
	
	return 0;
}
	
int DbBackend::db_end_transaction()
{
	int res;
		
	res = sqlite3_exec(db, "COMMIT",NULL,NULL,NULL);
	if(res != SQLITE_OK) {
		DB_PRINTERR("commit transaction error: ",db);
		return -1;
	}
	
	return 0;
}
