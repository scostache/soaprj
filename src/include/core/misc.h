/* 
 misc.h - Miscellaneous stuff related to string parsing, branch adding, etc.
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef MISC_H_
#define MISC_H_

#include <string>

#include "core/hybfs_data.hpp"
#include "core/path_crawler.hpp"

namespace hybfs {

/**
 * Pulls the real path from the path received as argument. This is useful
 * when having queries specified in the path, and we want to call the
 * underlying fs operation on our real path.
 * @param path the path sent by fuse
 * @param pc path parsing class, initialized before calling this function
 */
std::string* extract_real_path(const char *path, PathCrawler *pc);

/**
 * Transforms the path received into an absolute one. Used for adding new
 * branches
 * @param relpath the relative path
 */
std::string * make_absolute(const char *relpath);

/**
 * Transforms the relative path to the real one, so it can be used in ops from the
 * undelying fs. This implies a search in our branches, to see which one of them
 * has the path.
 * @param hybfs_core the HybFS data class that holds information about branches and dbs
 * @param path the relative path to our mount point
 * @param brid the branch id of our path
 * @ret the absolute path. It must be deleted after using it.
 */
std::string * resolve_path(HybfsData *hybfs_core, const char *path, int *brid);

/**
 * Breaks the tag:value pair in two separate strings: one for the tag, and the other one
 * for the value
 */
void break_tag(std::string *tag_value, std::string *tag, std::string *value);

/**
 * Extracts the tags from the specified query, and adds them to the vector tags.
 * Also, it gets the type of operation that needs to be performed on them.
 * Note that it will work only for a conjunction of tags, with the operation
 * specified as the first element (not considering the parenthesis).
 * 
 * @param query The query that needs to be parsed.
 * @param tags  The vector of tags in which the results will be put.
 * @param op_type The operation type. This can be '|' for appending all the tags,
 * '!' for deleting them, '+' or nothing for replacing them.
 */ 
int parse_tags(std::string *query, vector<std::string> *tags, int *op_type);

}

/**
 * Wrapper for the stat function. This will be used from the DB interface.
 */
extern int get_stat(const char *path, stat_t *buf);

extern void fill_dummy_stat(stat_t *st);

#endif /*MISC_H_*/
