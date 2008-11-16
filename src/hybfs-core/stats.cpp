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
	
	DBG_SHOWFC();
	DBG_PRINT("path: %s\n",path);
	
	res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
		
		return 0;
	}
	/* this is used for lookup, so check if
	 * the query is valid */
	
	/*res = vdir_validate(path, &flags);
	if(res && (!(flags & HAS_PATH)))
		return 0;
	*/
	flags = HAS_PATH;
	if(flags & HAS_PATH) {
		p = resolve_path(hybfs_core, path, &brid);
		res = lstat(p->c_str(), stbuf);
		
		delete p;
	}
	
	return res;
}

int hybfs_access(const char *path, int mask)
{
	int res;
	int flags, brid;
	std::string *p;
	
	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();
	
	if (strcmp(path, "/") == 0) {
		return 0;
	}

	/* this is to verify if the query expressed by path is valid */
	/*res = vdir_validate(path, &flags);
	if(res && (!(flags & HAS_PATH)))
		return 0;
	*/
	flags = HAS_PATH;
	
	if (flags & HAS_PATH) {
		/* if it's a normal path, not a query, then verify the real source*/
		p = resolve_path(hybfs_core, path, &brid);
		res = access(p->c_str(), mask);
		delete p;
	}
	
	if (res == -1)
		return -errno;

	return 0;
}
