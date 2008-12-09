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
#include <pthread.h>
#include <string>
#include <vector>

#include "db_backend.hpp"
#include "path_crawler.hpp"

typedef struct {
	int op;
	std::vector<std::string> tags;
} tags_op_t;



class VirtualDirectory{
private:
	DbBackend * db;
	
	std::string vdir_path;
	
public:
	VirtualDirectory(const char *path);
	
	~VirtualDirectory();
	
	/**
	 * Check if the initialization from the constructor went ok. If not,
	 * it returns -1.
	 */
	int check_for_init();
	/**
	 * Start the database associated with this virtual directory
	 */
	int init() { return db->db_init_storage(); }
	
	/**
	 * Checks if the query is valid. This means that all the tags from the
	 * query exist in the DB. We also set the flags value to indicate that we have
	 * a real path and/or tags.
	 * The return value is negative if one of the specified tags doesn't exist.
	 */
	int vdir_validate(const char *path, int *flags);
	
	/**
	 * Adds file info coresponding to this file, to the db.
	 */
	int vdir_add_tag(PathCrawler *pc, file_info_t *finfo);
	
	/**
	 *  Updates the tags for this path 
	 */
	int vdir_update_tags(PathCrawler *from, file_info_t *finfo);
	
	/**
	 * List root directory. This is special, because it lists all the tags 
	 * from the db.
	 */
	int vdir_list_root(const char * path,void *buf, fuse_fill_dir_t filler);
	
	/**
	 * Lists all file paths for a virtual directory.
	 */
	int vdir_readdir(const char * query, void *buf, fuse_fill_dir_t filler);
	
	/**
	 * Update the tags for a file. The type of update is given by the op parameter.
	 */
	int update_file(vector<string> *tags, int op, file_info_t *finfo, int exist);
	
	/**
	 * Replaces the tag-value components provided by the 'oldq' query with the ones
	 * provided by the 'newq' query. You should specify if the relative destination
	 * has a real component, so that the real rename from the unerlying fs will be
	 * called.
	 */
	int vdir_replace(const char*relfrom, const char *relto,
                         PathCrawler *from, PathCrawler *to, int do_fs_mv);
	
	/**
	 * Remove all info from the DB for this file.
	 */
	int vdir_remove_file(const char *path);
	
	/**
	 * Replaces the old path, given by from with the new one (to).
	 */
	int vdir_replace_path(const char *from, const char *to);
};

#endif /*VIRTUALDIR_H_*/
