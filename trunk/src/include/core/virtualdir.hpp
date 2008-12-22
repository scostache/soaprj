/* 
 virtualdir.h - Wrapper class for virtual directory operations. 
 		Interfaces with the DB backend.
 
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

namespace hybfs {

typedef struct {
	int op;
	std::vector<std::string> tags;
} tags_op_t;


/**
 * @class VirtualDirectory
 * @brief Wrapper class for virtual directory operations. 
 	  Interfaces with the DB backend (or wrapper).
 */

class VirtualDirectory{
private:
	/**
	 * The database interface pointer.
	 */
	DbBackend * db;
	
	/**
	 * The real path for the directory used by the database (temporary
	 * and permanent) files.
	 */
	std::string vdir_path;
	
public:
	VirtualDirectory(const char *path);
	
	~VirtualDirectory();
	
	/**
	 * @brief Checks if the initialization from the constructor went ok.
	 * You should use this after creating an object of type VirtualDirectory.
	 * @return  Returns -1 in case of error, and 0 if everything is ok.
	 */
	int check_for_init();
	
	/**
	 * @brief Start the database associated with the current virtual directory.
	 * @return Returns -1 in the case of an internal error.
	 */
	int init() { return db->db_init_storage(); }
	
	/**
	 * @brief Adds the associated metadata for this file, to the db.
	 * @return Returns -EINVAL in case of error and 0 for success.
	 * 
	 * @param[in] pc Path information wrapped in a PathCrawler class. We need this for
	 * extracting each query component from the path.
	 * @param[in] finfo File information structure.
	 */
	int vdir_add_tag(PathCrawler *pc, file_info_t *finfo);
	
	/**
	 * @brief Updates the tags for the file that has this path.
	 * @return  -EINVAL in case of error and 0 if is successful.
	 * @param[in] from Path information wrapped in a PathCrawler class.
	 * We need this for extracting each query component from the path.
	 * @param[in] finfo File information structure.
	 */
	int vdir_update_tags(PathCrawler *from, file_info_t *finfo);
	
	/**
	 * @brief Lists the root or the real directory. 
	 * This is special, because it lists all the tags and the tag:value pairs
	 * for this path.
	 * @return Returns 0 for success and !=0 otherwise.
	 * 
	 * @param[in] path Directory path.
	 * @param[in] buf  The buffer received from the caller. It must be filled
	 * by the filler function.
	 * @param[in] filler The filler function - this is opaque, for portability.
	 */
	int vdir_list_root(const char * path,void *buf, fuse_fill_dir_t filler);
	
	/**
	 * @brief Lists all file paths for a virtual directory specified by a query.
	 * @return Returns 0 for success and !=0 otherwise.
	 * 
	 * @param[in] query This can contain a query, a conjunction of queries and/or
	 * a real path.
	 * @param[in] buf  The buffer received from the caller. It must be filled
	 * by the filler function.
	 * @param[in] filler The filler function - this is opaque, for portability.
	 */
	int vdir_readdir(const char * query, void *buf, fuse_fill_dir_t filler);
	
	/**
	 * @brief Update the tags for a file. The type of update is given by the 
	 * op and exist parameters.
	 * @return Returns 0 for success and !=0 otherwise.
	 * 
	 * @param[in] tags The tags that will be added/replace the other tags, or be deleted.
	 * @param[in] op   The type of operation. This can be add, replace or remove.
	 * @param[in] finfo The structure that holds info about the file.
	 * @param[in] exist If this parameter has a non-zero value then the information related
	 * to the file is already introduced in the DB and no other checks will be necessary.
	 */
	int update_file(vector<string> *tags, int op, file_info_t *finfo, int exist);
	
	/**
	 * @brief Replaces the tag-value components provided by the 'oldq' 
	 * query with the ones provided by the 'newq' query.
	 * You should specify if the relative destination has a real component,
	 * so that the real rename from the unerlying fs will be called.
	 * \par
	 * This is not (yet) implemented.
	 */
	int vdir_replace(const char*relfrom, const char *relto,
                         PathCrawler *from, PathCrawler *to, int do_fs_mv);
	
	/**
	 * @brief Removes all info from the DB for this file.
	 * @return Returns 0 for success and !=0 otherwise.
	 * 
	 * @param[in] path The file path.
	 */
	int vdir_remove_file(const char *path);
	
	/**
	 * @brief Replaces the old path, given by from with the new one (to).
	 * @return Returns 0 for success and !=0 otherwise.
	 * 
	 * @param[in] from The file path to be replaced.
	 * @param[in] to   The new file path that will replace the old one.
	 */
	int vdir_replace_path(const char *from, const char *to);
};

}

#endif /*VIRTUALDIR_H_*/
