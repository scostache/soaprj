/*
 hybfs_ops.hpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef HYBFS_OPS_HPP_
#define HYBFS_OPS_HPP_

#include "core/db_backend.hpp"

using namespace hybfs;

using namespace std;

/** structure to associate a database with a specific path */
struct db_assoc
{
	DbBackend *db;
	const char * path;
};


class HybFSOps
{
private:
	//DbBackend *db;
	vector<db_assoc> vect_db;
public:
	HybFSOps();
	~HybFSOps();

	/** the database file */
	int ops_load_db(const char * path);

	list<string>* ops_list_tags(const char *path);

	list<string>* ops_list_file_tags(const char *path);

	int ops_file_add_tag(const char *tag, const char *path);

	/** removes a tag:value associated with a file
	 * @param path File path
	 * @param tag The tag string
	 * @param value The value string
	 */
	int ops_file_remove_tag(const char *tag, const char *path);

	int ops_copy_file(const char *src, const char *dst);

	DbBackend * get_database(const char *path);

	const char * get_value(const char *str);

	const char * get_tag(const char *str);

	int ops_read_conf_file (FILE * f);

	int verify_database(const char *path);

	/** lists all the virtual directory paths */
	vector<string> * ops_list_db();

	/** obtains the root path for the virtual dir
	 * within which the file resides
	 * @param path The file path
	 */
	const char * get_vdir_path(const char *path);

	/* extract the file information to copy the file
	 * @param path The file path
	 */
	file_info_t * get_file_info(const char * path);
};

#endif /* HYBFS_OPS_HPP_ */
