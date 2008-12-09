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
 * Default meta dir path. Define it at compile time if you want to change it.
 */
#ifndef METADIR
#define METADIR ".hybfs/"
#endif

/**
 * Default databases names. Define them at compile time if you want to change them.
 */
#ifndef MAINDB
#define MAINDB  ".hybfs_main.db"
#endif


using namespace std;

typedef struct{
	int ino;
	string path;
} new_file_info_t;

/**
 * @class Wrapper class for the interface with the DB.
 * 
 * The structure of the information in the Sqlite3 database:
 * 
 * table tags: tag_id primary hey (autoincremented number)
 * 		tag, value
 * 
 * table files: ino primary key, mode, path, tags (string of tags:values)
 * 
 * table assoc: (ino, tag_id) primary key
 */

class DbBackend{
private:
	/**
	 * Wrapper for running a query. This is mostly used for creating tables.
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
	 * Adds all the tags from the vector 'tags' to the database, having the 
	 * association made for the file described by the structure 'finfo'
	 */
	int db_add_tag_info(vector<string> *tags, file_info_t * finfo, int behaviour);
	/**
	 * Adds the information about a file to the database. It does not replace current info.
	 * It returns 0 for succes. Note that in the case of a duplicate ino it returns error.
	 */
	int db_add_file(file_info_t * finfo);
	
	int fill_files(string *path, string *temp_table, void *buf, filler_t filler);
	
	string *build_temp_table(string *query, string *path);
	
	int delete_temp_table(string *name);
	
	/**
	 * path to the database
	 */
	string  db_path;
	
	/**
	 * path of our branch
	 */
	string vdir_path;
	
	/**
	 * handle to the database 
	 */
	sqlite3 *db;
	
	/* cached requests structures should be here */
	
public:
	
	DbBackend(const char * path, const char *vdir_path);
	
	~DbBackend();
	
	/**
	 * Initializes the database: create initial database file and tables in the
	 * desired branch, if they don't exist.
	 */
	int db_init_storage();
	
	/**
	 * Closes the database.
	 */
	void db_close_storage();
	
	/**
	 * Adds the file information for a list of tags in the main db.
	 * 
	 * @param tags The tags that will be added for this file. If the tag has
	 * the form tag:value then the value will be extracted and added in the db also.
	 * Otherwise, it will be replaced with the 'null' value. Note that a tag can have
	 * more than one value associated.
	 * 
	 * @param finfo The file info structure. It contains the relative file path, mode
	 * and ino.
	 * @param exist If the file info exist in the DB and you are shure of that, than
	 * the process of adding info about it can be skiped and only the tags will be added.
	 */
	int db_add_file_info(vector<string> *tags, file_info_t * finfo, int exist);
	
	/**
	 * Deletes the records from the DB for the file with the absolute path "abspath".
	 * 
	 * @param path The relative file path.
	 */
	int db_delete_file_info(const char *abspath);
	
	/**
	 * Deletes the tag for this file, from the db. If a value is specified, then is
	 * replaced with null in the BD. This means I delete the value for this tag.
	 * Otherwise, it deletes the tag association for the file given by path.
	 * 
	 * @param tag  The tag string.
	 * @param value The value string.
	 * @param path The relative file path.
	 */
	int db_delete_file_tag(const char *tag, const char *value, const char *path);
	
	/**
	 * Deletes all the tags specified in the vector tags. They can be pairs of 
	 * 'tag:value' and then only the values for these tags will be deleted.
	 */
	int db_delete_file_tags(vector<string> *new_tags, file_info_t *finfo);
	/**
	 * Updates the tags for this file, from the db. Actually it replaces the old
	 * ones with the tags specified.
	 * 
	 * @param new_tags The new tags for this file.
	 * @param finfo    The structure that contains information about the file.
	 * @param exist    If you are shure that the files exist in the DB, then set this
	 * param to 1. Unnecessary checking will be avoided. 
	 */
	int db_update_file_tags(vector<string> *new_tags, file_info_t *finfo, int exist);
	
	/**
	 * Updates the file path from the db, in the case of a rename
	 */
	int update_file_path(const char *from, const char *to);
	
	/**
	 * Checks if the tag is really a key in the main DB. Returns the tag id
	 * if it exists, 0 otherwise. In case of error, returns -1.
	 * 
	 * @param tag The tag string
	 * @param value The value of this tag. If it's NULL than the 'null' value
	 * will be used.
	 */
	int db_check_tag(const char *tag, const char *value);	
	
	/**
	 * Checks if a file exists in the database. Returns 0 in case of an error or
	 * if the file path doesn't exist.
	 * 
	 * @param path The relative file path.
	 */
	int db_check_file(const char *path);
	
	/**
	 * Returns all tag-value pairs from the database for a path.
	 * 
	 * @param path The relative file path. If the path
	 * is NULL then all the tags are returned.
	 */
	list<string> * db_get_tags_values(const char *path);
	
	/**
	 * Returns all tags from the database for a path. If the path is NULL
	 * then all the tags are returned.
	 */
	list<string> * db_get_tags(const char *path);
	
	/**
	 * Returns the file information for the tag and the specified value.
	 * If the value is null than it returns all the files that have that tag.
	 * If the path is specified, it will try to do a match with the real path 
	 * from the database.
	 * 
	 * @param path The relative file path. If it is NULL then it will be ommited.
	 * @param tag The tag string
	 * @param value The tag value
	 * @param buf The buffer that will be filled with the info related to the files.
	 * @param filler The function that will fill the info from the buffer.
	 */
	int db_get_files(const char * path, const char * tag, const char *value,
	                 void * buf, filler_t filler);

	
	int db_get_filesinfo(string *query, vector<tag_info_t> *tags,string *path, 
	                     void * buf, filler_t filler);
	
	int get_file_names(string *query, string *path, vector<new_file_info_t> *files);
	
	/**
	 * Helper functions for transactions on the DB. Use them carefully
	 */
	
	/**
	 * This starts a transation on the DB.
	 */
	int db_begin_transaction();
	
	/**
	 * This performs rollback - you'll need this in the case on an error.
	 */
	int db_rollback();
	
	/**
	 * This commits a transaction.
	 */
	int db_end_transaction();
};

#endif /*DB_BACKEND_HPP_*/
