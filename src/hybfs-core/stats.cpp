/*
 stats.c - file attributes and stats
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <string>

#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "hybfs.h"
#include "misc.h"


int hybfs_getattr(const char *path, struct stat *stbuf)
{
	int res, flags;
	int brid;
	std::string *p;

	HybfsData *hybfs_core = get_data();

	assert(hybfs_core);
	assert(path);
	assert(stbuf);

	DBG_SHOWFC();
	DBG_PRINT("path: %s\n", path);

	res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (strncmp(path, "/",1) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;

		return 0;
	}
	
	if(strcmp(path+1, REAL_DIR) == 0) {
		/* this is expensive */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2 + hybfs_core->get_nlinks();
		stbuf->st_uid = geteuid();
		stbuf->st_gid = getegid();
	}
	
	/* TODO this is used for lookup, so check if
	 * the query is valid */

	flags = HAS_PATH;
	try {
		if(flags & HAS_PATH) {
			p = resolve_path(hybfs_core, path+1+strlen(REAL_DIR), &brid);
			if(p==NULL)
			throw std::bad_alloc();

			res = lstat(p->c_str(), stbuf);

			delete p;
		}
	}
	catch (std::exception) {
		PRINT_ERROR("Hybfs Internal error in func %s line %d\n",
				__func__,__LINE__);
		return -EIO;
	}

	return res;
}

int hybfs_access(const char *path, int mask)
{
	int res, path_len;
	int flags, brid;
	std::string *p;
	
	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();
	
	if (strcmp(path, "/") == 0)
		return 0;

	/* hack */
	path_len = strlen(path) -1;
	if (path_len == (strlen(REAL_DIR) - 1) && strncmp(path, REAL_DIR,
	                path_len))
		return 0;
	if (strcmp(path+1, REAL_DIR) == 0)
		return 0;

	/* TODO: this is to verify if the query expressed by path is valid */
	
	flags = HAS_PATH;
	try {
	if (flags & HAS_PATH) {
		/* if it's a normal path, not a query, then verify the real source*/
		p = resolve_path(hybfs_core, path+path_len+1, &brid);
		if(p==NULL)
			throw std::bad_alloc();
		
		res = access(p->c_str(), mask);
		delete p;
	}
	}
	catch (std::exception) {
		PRINT_ERROR("Hybfs Internal error in func %s line %d\n",
				__func__,__LINE__);
		return -EIO;
	}
	
	if (res == -1)
		return -errno;

	return 0;
}
