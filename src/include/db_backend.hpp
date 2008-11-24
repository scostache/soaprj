/* 
 db_backend.hpp - Wrapper class for the database access
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef DB_BACKEND_HPP_
#define DB_BACKEND_HPP_

#include <string>
#include <list> 

#include <sqlite3.h>
#include "hybfsdef.h"

/**
 *meta dir path
 */
#ifndef METADIR
#define METADIR ".hybfs/"
#endif

/**
 * databases names 
 */
#ifndef MAINDB
#define MAINDB  ".hybfs_main.db"
#endif

using namespace std;

/**
 * @class Wrapper class for the interface with the DB.
 * 
 * The structure of the information in the Sqlite3 database:
 * 
 * table tags: tag_id primary hey (autoincremented number)
 * 		tag, value
 * 
 * table files: ino primary key, mode, path
 * 
 * table assoc: (ino, tag_id) primary key
 */

class DbBackend{
private:
	/**
	 * wrapper for running a query. This is mostly used for creating tables.
	 */
	int run_simple_query(const char* query);
	/**
	 * Creates initial tables for the given database
	 */
	int create_main_tables();
	
	/**
	 * Adds a pair (tag,value) to the "tags" table. Returns the associated 
	 * unique number. It does not replace the value for an existing tag.
	 * For internal use only.
	 */
	int db_add_tag(const char *tag, const char *value);
	
	/**
	 * Adds the information about a file to the database. It does not replace current info.
	 * It returns 0 for succes. Note that in the case of a duplicate ino it returns error.
	 */
	int db_add_file(file_info_t * finfo);
	
	/**
	 * path to the database
	 */
	string  db_path;
	/**
	 * handle to the database 
	 */
	sqlite3 *db;
	
	/* cached requests structures should be here */
	
public:
	
	DbBackend(const char * path);
	
	~DbBackend();
	
	/**
	 * Initialize database: create initial database file and tables in the
	 * desired branch, if they don't exist.
	 */
	int db_init_storage();
	
	/**
	 * Close all the databases here
	 */
	void db_close_storage();
	
	/**
	 * Adds the file information for a list of tags in the main db.
	 */
	int db_add_file_info(vector<string> *tags, file_info_t * finfo);
	
	/**
	 * Deletes all the records from the main DB, coresponding to the key "tag"
	 */
	int db_delete_allfile_info(char *tag);
	
	/**
	 * Deletes the record with the absolute path "abspath"
	 */
	int db_delete_file_info(char *abspath);
	
	/**
	 * Deletes all the information related to this tag
	 */
	int db_delete_file_tag(char *tag);
	
	/**
	 * Checks if the tag is really a key in the main DB. Returns the tag id
	 * if it exists, 0 otherwise. In case of error, returns -1.
	 */
	int db_check_tag(const char *tag, const char *value);	
	
	/**
	 * Returns all tag-value pairs from the database
	 */
	list<string> * db_get_tags_values();
	
	/**
	 * Returns all tags from the database
	 */
	list<string> * db_get_tags();
	
	/**
	 * Returns the file information for the tag and the specified value.
	 * If the value is null than it returns all the files that have that tag.
	 * If the path is specified, it will try to do a match with the real path 
	 * from the database.
	 */
	int db_get_files(const char * path, const char * tag, const char *value,
	                 void * buf, filler_t filler);

	int db_get_filesinfo(list<string> *tags, string *path, void * buf, filler_t filler);
};

#endif /*DB_BACKEND_HPP_*/
