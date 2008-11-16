/* 
 db_ops.c - Berkley DB access methods
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "db_backend.h"
#include "hybfs.h"
#include "hybfsdef.h"
#include "misc.h"

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
	db_path = path;
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
			"tag_id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
			"tag VARCHAR(256), \n"
			"value VARCHAR(256) \n );");
		DB_ERROR(ret != SQLITE_OK,"Table TAGS ", db);

		ret = run_simple_query("CREATE TABLE files("
			"ino INTEGER PRIMARY KEY, \n"
			"mode INTEGER, \n"
			"path VARCHAR(256), \n"
			"name VARCHAR(256) \n );");
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
		return 1;
	}

	return 0;
}

int DbBackend::db_add_file_info(char *tag, char *value, file_info_t * finfo)
{
	int ret = 0;

	DBG_SHOWFC();

	/* adds the info in all the tables */
	/*	ret = sqlite3_prepare_v2(db,"INSERT INTO tags VALUES (NULL, ?1, ?2);"
	 , -1, &select, 0);
	 if (ret != SQLITE_OK || !select) {
	 DB_PRINTERR("Preparing select: ",hybfs_db.db);
	 goto error;
	 }
	 sqlite3_bind_text(select, 1, tag, -1, SQLITE_TRANSIENT);
	 sqlite3_bind_text(select, 2, value, -1, SQLITE_TRANSIENT);
	 ret = sqlite3_step(select);
	 sqlite3_finalize(select);
	 
	 ret = sqlite3_prepare_v2(db,"INSERT INTO files VALUES (?1, ?2, ?3, ?4);"
	 , -1, &select, 0);
	 if (ret != SQLITE_OK || !select) {
	 DB_PRINTERR("Preparing select: ",hybfs_db.db);
	 goto error;
	 }
	 
	 sqlite3_bind_int(select, int, int);

	 */
	return ret;
}

int DbBackend::db_get_file_info(char *tag, file_info_t **finfo)
{
	int ret = 0;

	DBG_SHOWFC();

	return ret;
}

int DbBackend::db_delete_allfile_info(char *tag)
{
	int ret = 0;

	return ret;
}

int DbBackend::db_delete_file_info(char *tag, char *abspath)
{
	int ret = 0;

	return ret;
}

int DbBackend::db_check_tag(char *tag)
{
	int res = 0;

	/* suppose I try to do a get with the key "tag" */

	return res;
}
