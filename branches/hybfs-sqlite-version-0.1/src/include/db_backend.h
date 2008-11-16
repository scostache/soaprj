/* 
 db_backend.h - Wrapper class for the database access
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef DB_OPS_H_
#define DB_OPS_H_

#include <string>

#include <sqlite3.h>
#include "hybfsdef.h"

/* meta dir path*/
#ifndef METADIR
#define METADIR ".hybfs/"
#endif

/*  databases names */
#ifndef MAINDB
#define MAINDB  ".hybfs_main.db"
#endif

using namespace std;


class DbBackend{
private:
	/*
	 * wrapper for running a query. This is mostly used for
	 * creating tables.
	 */
	int run_simple_query(const char* query);
	/*
	 * Creates initial tables for the given database
	 */
	int create_main_tables();
	
	/* path to the database */
	string  db_path;
	/* handle to the database */
	sqlite3 *db;
	
	/* cached requests structures should be here */
	
public:
	
	DbBackend(const char * path);
	
	~DbBackend();
	
	/*
	 * Initialize database: create initial database file and tables
	 * in the desired branch, if they don't exist.
	 */
	int db_init_storage();
	
	/*
	 * Close all the databases here
	 */
	void db_close_storage();
	
	/*
	 * Adds the file information for a tag in the main db. The tag represents
	 * the key of this DB. You also have to specify a value.
	 */
	int db_add_file_info(char *tag, char *value, file_info_t * finfo);
	
	/*
	 * Retreives the file information for the current position of the cursor.
	 * The cursor must be opened before calling this function, and closed when
	 * finish using it.
	 * The data is stored in finfo.
	 */
	int db_get_file_info(char *tag, file_info_t **finfo);
	
	/*
	 * Deletes all the records from the main DB, coresponding
	 * to the key "tag"
	 */
	int db_delete_allfile_info(char *tag);
	
	/*
	 * Deletes the record with the absolute path "abspath"
	 */
	int db_delete_file_info(char *abspath);
	
	/*
	 * Deletes the record with the file identifier ino
	 */
	int db_delete_file_info(int ino);
	
	/*
	 * Deletes all the information related to this tag
	 */
	int db_delete_file_tag(char *tag);
	
	/* 
	 * Checks if the tag is really a key in the main DB.
	 * Returns 1 if it exists, 0 otherwise.
	 */
	int db_check_tag(char *tag);	
};

#endif /*DB_OPS_H_*/
