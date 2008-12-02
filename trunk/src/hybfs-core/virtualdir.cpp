/* 
 virtualdir.cpp - Wrapper for virtual directory operations
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <vector>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <fuse.h>

#include <boost/tokenizer.hpp>

#include "misc.h"
#include "hybfs.h"
#include "hybfsdef.h"

#include "db_backend.hpp"
#include "virtualdir.hpp"
#include "path_crawler.hpp"

typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;


VirtualDirectory::VirtualDirectory(const char *path)
{
	struct stat buf;
	string abspath = path;
	abspath.append(METADIR);
	db = NULL;
	
	/* check if the directory exists, if not - create */
	if (lstat(abspath.c_str(), &buf) == -1) {
		DBG_PRINT("Error at checking directory! Atempt to create one!\n");
		/* TODO get appropriate permisions for our directory */
		if (mkdir(abspath.c_str(), 0755)) {
			perror("Failed to create directory: ");
		}
	}
	abspath.append(MAINDB);

	db = new DbBackend(abspath.c_str());
}

VirtualDirectory::~VirtualDirectory()
{
	delete db;
}

int VirtualDirectory::check_for_init()  {
	return (db == NULL) ? -1 :0;
}

int VirtualDirectory::vdir_add_tag(vector <string> *tags, file_info_t *finfo)
{
	int res;

	/* bad, bad, bad: accept only the absolute path for a file */
	if(finfo->mode & S_IFDIR)
		return -EISDIR;
	
	res = db->db_add_file_info(tags, finfo);
	if(res)
		res = -EINVAL;

	return res;
}

int VirtualDirectory::vdir_remove_file(const char *path)
{
	int res;
	const char *lpath;
	
	if(path == NULL)
		return -EINVAL;
	if(path[0] == '\0')
		return -EINVAL;
	
	lpath = path;
	if(path[0] == '/')
		lpath = path + 1;
	
	res = db->db_delete_file_info(lpath);
	if(res)
		res = -EINVAL;
	
	return res;
}

int VirtualDirectory::vdir_remove(const char *query)
{
	int res = 0;
	
	return res;
}

int VirtualDirectory::vdir_replace(const char * oldq, const char *newq)
{
	int res = 0;
	
	/* TODO parse the queries, get the lists of tags to be changed
	 * and the last thing: call the db routine to change them in the DB. */
	return res;
	
}


static int fill_buf(list<string> *tags, void *buf, filler_t filler)
{
	for (list<string>::const_iterator i = tags->begin(); i != tags->end(); ++i) {
		if (filler(buf, (*i).c_str(), NULL, 0))
			return 1;
	}

	return 0;
}

int VirtualDirectory::vdir_list_root(const char * path, void *buf, filler_t filler)
{
	list<string> *tags;
	int res = 0;
	
	try {
		/* get all the simple tags */
		tags = db->db_get_tags(path);
		if(tags == NULL)
			throw std::bad_alloc();
		res = fill_buf(tags, buf, filler);
		tags->clear();
		delete tags;
		if(res)
			return 0;
	
		/* and all the tag:value pairs */
		tags = db->db_get_tags_values(path);
		if(tags == NULL)
			throw std::bad_alloc();
	
		res = fill_buf(tags, buf, filler);
		tags->clear();
		delete tags;
	}
	catch(std::exception) {
		PRINT_ERROR("Hybfs: Not enough memory! %s line %d\n",
				__func__,__LINE__);
		return -ENOMEM;
	}
	/* res is the result from our filler*/
	
	return 0;
}

int VirtualDirectory::vdir_readdir(const char * query, void *buf,
                                   filler_t filler)
{
	list<string> *tags;
	int res = 0;
	PathCrawler *pc;
	string *path_query;

	/* is this an empty query? */
	if (query[0] == '\0') {
		PRINT_ERROR("NULL query; use readroot instead! \n");
		return -EINVAL;
	}

	tags = new list<string>;
	
	/* get all the queries that are relative to this one in a queue */
	pc = new PathCrawler(query);
	if(pc == NULL) {
		delete tags;
		
		return -ENOMEM;
	}
	/* we have a path like /dir/dir1/.. and it's invalid */
	if(pc->break_queries() == 0) {
		delete tags;
		delete pc;
		
		return -ENOENT;
	}
	
	/* TODO : here I should rebuild the query, so that '/' becomes + ? 
	 * um, and use the parser so that we can support complex queries */
	res = 0;
	path_query = extract_real_path(query, pc);
	
	if(path_query != NULL)
	DBG_PRINT("first path is %s\n", path_query->c_str());
	
	while(pc->has_next_query()) {
		string to_query = pc->pop_next_query();
		/* strip the '(' and ')' */
		try {
			to_query.erase(0,1);
			to_query.erase(to_query.length() -1);
		}
		catch(std::exception) {
			DBG_PRINT("Weird exception at parsing string %s\n", 
					to_query.c_str());
			res = -1;
			break;
		}
		DBG_PRINT("query path is %s \n", to_query.c_str());
		tags->push_back(to_query);
	}
	/* now I have the tag:value packed in a list, get us some results.
	 * Fill directly the buffer using the provided filler method */
	if(res == 0)
		res = db->db_get_filesinfo(tags, path_query, buf, filler);
	
	tags->clear();
	delete tags;
	delete pc;
	if(path_query)
		delete path_query;
	
	return (res == -1) ? -EIO : 0;
}
