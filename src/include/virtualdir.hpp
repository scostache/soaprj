/* 
 virtualdir.h - Wrapper class for virtual directory operations. 
 		Interfaces with the (non-existent) parser.
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef VIRTUALDIR_H_
#define VIRTUALDIR_H_

#include <fuse.h>

#include "db_backend.hpp"

class VirtualDirectory{
private:
	DbBackend * db;
	
public:
	VirtualDirectory(const char *path);
	
	~VirtualDirectory();
	
	/*
	 * Start the database associated with this virtual directory
	 */
	int init() { return db->db_init_storage(); }
	
	/*
	 * Checks if the query is valid. This means that all the tags from the
	 * query exist in the DB. We also set the flags value to indicate that we have
	 * a real path and/or tags.
	 * The return value is negative if one of the specified tags doesn't exist.
	 */
	int vdir_validate(const char *path, int *flags);
	
	/*
	 * Adds file info coresponding to this file, to the db.
	 */
	int vdir_add_tag(vector <string> *tags, const char *path);
	
	/*
	 * List root directory. This is special, because it lists all the tags from the db.
	 */
	int vdir_list_root();
	
	/*
	 * readdir for a virtual directory
	 */
	int vdir_readdir(const char * query, void *buf, fuse_fill_dir_t filler);
	
	/*
	 * replaces the tag-value components provided by the 'oldq' query with the ones
	 * provided by the 'newq' query.
	 */
	int vdir_replace(const char * oldq, const char *newq);
	
};

#endif /*VIRTUALDIR_H_*/
