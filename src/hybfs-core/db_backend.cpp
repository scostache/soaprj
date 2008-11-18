/* 
 db_backend.cpp - database access methods
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */
#include <sstream>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "db_backend.h"
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
	DBG_SHOWFC();

	/* close all databases here */
	if (!db)
		return;

	sqlite3_close(db);
	db = NULL;
}

int DbBackend::run_simple_query(const char* query)
{
	int ret;
	sqlite3_stmt *select= NULL;

	ret = sqlite3_prepare_v2(db, query, -1, &select, 0);
	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}
	ret = sqlite3_step(select);
	if (ret != SQLITE_DONE) {
		DB_PRINTERR("Running select: ",db);
		goto error;
	}

	ret = sqlite3_finalize(select);

	return ret;

	error: if (select)
		sqlite3_finalize(select);
	return 1;
}

int DbBackend::create_main_tables()
{
	int ret;
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

	ret = sqlite3_step(select);
	if (ret != SQLITE_DONE && ret != SQLITE_ROW) {
		DB_PRINTERR("Database error: ",db);
		sqlite3_finalize(select);
		return -1;
	}

	/* we have an empy database, create the tables */
	if (ret == SQLITE_DONE) {
		DBG_PRINT("HYBFS: Creating main table \n");
		sqlite3_finalize(select);

		ret = run_simple_query("CREATE TABLE tags("
			"tag VARCHAR(256), \n"
			"value VARCHAR(256) \n"
			"file_ino INTEGER, \n"
			" PRIMARY KEY (tag, value);");
		DB_ERROR(ret != SQLITE_OK,"Table TAGS ", db);

		ret = run_simple_query("CREATE TABLE files("
			"ino INTEGER PRIMARY KEY, \n"
			"mode INTEGER, \n"
			"path VARCHAR(256), \n"
			"name VARCHAR(256) \n );");
		DB_ERROR(ret != SQLITE_OK,"Table FILES ", db);
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
		return 1;
	}

	return 0;
}

int DbBackend::db_add_tag(char *tag, char *value)
{
	int ret = -1;
	sqlite3_stmt *select= NULL;

	DBG_SHOWFC();

	/* adds the info in all the tables */
	ret = sqlite3_prepare_v2(db, "INSERT INTO tags VALUES (NULL, ?1, ?2);",
	                -1, &select, 0);

	if (ret != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing insert: ",db);
		goto error;
	}

	sqlite3_bind_text(select, 1, tag, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(select, 2, value, -1, SQLITE_TRANSIENT);
	sqlite3_step(select);
	
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

int DbBackend::db_add_file_info(char *tag, char *value, file_info_t * finfo)
{
	int ret = 0;
	int tag_id;
	
	sqlite3_stmt *select= NULL;
	
	DBG_SHOWFC();

	/* adds the tag info */
	tag_id = db_add_tag(tag, value);
	if(tag_id <= 0)
		goto error;
	
	/* now adds the file info */

	
	return 0;
error: 
	ret = -1;
	if (select)
		sqlite3_finalize(select);

	return ret;
}

int DbBackend::db_delete_file_tag(char *tag)
{
	int ret = 0;

	/* TODO */
	
	return ret;
}

int DbBackend::db_delete_file_info(char *abspath)
{
	/* TODO */
	
	return 0;
}

int DbBackend::db_delete_file_info(int ino)
{
	/* TODO */
	
	return 0;
}

int DbBackend::db_get_file_info(char *tag, file_info_t **finfo)
{
	int ret = 0;

	DBG_SHOWFC();

	/* TODO */
	
	return ret;
}

int DbBackend::db_delete_allfile_info(char *tag)
{
	int ret = 0;

	/* TODO */
	
	return ret;
}

int DbBackend::db_check_tag(char *tag, char *value)
{
	int res = 0;
	int tag_id;

	sqlite3_stmt *select= NULL;

	DBG_SHOWFC();

	/* search for the tag id in the table */
	res = sqlite3_prepare_v2(db, "SELECT tag_id FROM tags "
			" WHERE  tag == ?1 AND value == ?2 ;",
	                -1, &select, 0);

	if (res != SQLITE_OK || !select) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}

	sqlite3_bind_text(select, 1, tag, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(select, 2, value, -1, SQLITE_TRANSIENT);
	res = sqlite3_step(select);
	switch(res){
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
	if(tag_id < 0)
		goto error;
	
	res = sqlite3_finalize(select);
	
	return (res != SQLITE_OK) ? -1 :  tag_id;
	
error:
	if(select)
		sqlite3_finalize(select);
	return -1;
}


static int tags_callback(void *data, int argc, char **argv, char **colname)
{
	vector<string> *tags = (vector<string> *) data;

	tags->push_back(string(colname[0]));
	
	return 0;
}


vector<string> * DbBackend::db_get_tags()
{
	int res = 0;
	char * err;
	vector<string> *tags = new vector<string>;

	DBG_SHOWFC();
	
	res = sqlite3_exec(db, "SELECT tag FROM tags ;", tags_callback, tags, &err);
	if( res !=SQLITE_OK ){
	    PRINT_ERROR("SQL error: %s\n", err);
	    sqlite3_free(err);
	    
	    tags->clear();
	    delete tags;
	    
	    return NULL;
	  }
	
	return tags;
}

vector<string> * DbBackend::db_get_files(const char * tag, const char *value)
{
	vector<string> *tags = new vector<string>;
	sqlite3_stmt* sql;
	int res;

	string sql_string = "SELECT path,name FROM files  WHERE tags.tag = ?1";
	if (value != NULL)
		sql_string.append(" AND tags.value = ?2 ");
	sql_string.append("INNER JOIN tags ON tags.tag_id=files.tag_id ;");

	res = sqlite3_prepare_v2(db, sql_string.c_str(), -1, &sql, 0);
	if (res != SQLITE_OK || !sql) {
		DB_PRINTERR("Preparing select: ",db);
		goto error;
	}

	sqlite3_bind_text(sql, 1, tag, -1, SQLITE_TRANSIENT);
	if (value != NULL)
		sqlite3_bind_text(sql, 2, value, -1, SQLITE_TRANSIENT);

	while ((res = sqlite3_step(sql)) == SQLITE_ROW) {
		string abspath = (const char *) sqlite3_column_text(sql, 0);
		abspath += (const char *) sqlite3_column_text(sql, 1);

		tags->push_back(abspath);

		res = sqlite3_step(sql);
	}

	if (res != SQLITE_DONE) {
		DB_PRINTERR("Error at processing select: ",db);
		goto error;
	}
	res = sqlite3_finalize(sql);
	if (res != SQLITE_OK) {
		sql = NULL;
		DB_PRINTERR("Error at finalizing select: ",db);
		goto error;
	}

	return tags;

error: 
	if (sql)
		sqlite3_finalize(sql);

	delete tags;

	return NULL;
}
