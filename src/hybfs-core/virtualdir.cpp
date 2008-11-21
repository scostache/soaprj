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

typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;


VirtualDirectory::VirtualDirectory(const char *path)
{
	db = new DbBackend(path);
}

VirtualDirectory::~VirtualDirectory()
{
	delete db;
}

int VirtualDirectory::vdir_add_tag(vector <string> *tags, const char *path)
{
	struct stat stbuf;
	file_info_t *finfo;
	int len, brid;
	int res;

	memset(&stbuf, 0, sizeof(struct stat));
	len = strlen(path);

	res = lstat(path, &stbuf);
	if (res)
		return -errno;
	
	/* bad, bad, bad: accept only the absolute path for a file */
	if(stbuf.st_mode & S_IFDIR)
		return -EISDIR;
	
	finfo = (file_info_t*) malloc(sizeof(file_info_t)+len+1);
	finfo->brid = brid;
	finfo->fid = stbuf.st_ino;
	finfo->mode = stbuf.st_mode;
	finfo->namelen = len;
	memcpy(&finfo->name[0], path, len);

	db->db_add_file_info(tags, finfo);


	free(finfo);

	return res;
}

int VirtualDirectory::vdir_replace(const char * oldq, const char *newq)
{
	int ret = 0;
	
	/* TODO parse the queries, get the lists of tags to be changed
	 * and the last thing: call the db routine to change them in the DB. */
	return ret;
	
}


static int fill_buf(list<string> *tags, void *buf, fuse_fill_dir_t filler)
{
	for (list<string>::const_iterator i = tags->begin(); i != tags->end(); ++i) {
		if (filler(buf, (*i).c_str(), NULL, 0))
			return 1;
	}

	return 0;
}

int VirtualDirectory::vdir_readdir(const char * query, void *buf,
                                   fuse_fill_dir_t filler)
{
	list<string> *tags;
	int res = 0;

	/* is this an empty query? If yes, then fill with all the tags from the
	 * db, but no files*/
	if (query[0] == '\0') {
		try {
			/* get all the simple tags */
			tags = db->db_get_tags();
			if(tags == NULL)
				throw std::bad_alloc();
			res = fill_buf(tags, buf, filler);
			tags->clear();
			delete tags;
			if(res)
			return 0;

			/* and all the tag:value pairs */
			tags = db->db_get_tags_values();
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
		if (res)
			return 0;
	}

	/* no, it is not empy -> then get all the queries that are relative
	 * to this one in a queue */

	return res;
}
