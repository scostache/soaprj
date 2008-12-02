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
#include "path_crawler.hpp"


static inline int normal_getattr(HybfsData *data, const char *path,
                                 struct stat *stbuf)
{
	std::string *p;
	int brid;
	int res = 0;

	p = resolve_path(data, path+strlen(REAL_DIR), &brid);
	if (p==NULL)
		return -ENOMEM;

	DBG_PRINT("my path is %s\n", p->c_str());

	res = lstat(p->c_str(), stbuf);
	if (res)
		res = -errno;
	
	delete p;

	return res;
}

int hybfs_getattr(const char *path, struct stat *stbuf)
{
	int res, nq;
	int brid;
	std::string *p;
	std::string first, last;
	PathCrawler *pc= NULL;

	HybfsData *hybfs_core = get_data();

	assert(hybfs_core);
	assert(path);
	assert(stbuf);

	DBG_SHOWFC();
	DBG_PRINT("path: %s\n", path);

	res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	
	if (strcmp(path, "/") == 0 || strcmp(path+1, REAL_DIR) == 0) {
		/* this is expensive */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2 + hybfs_core->get_nlinks();
		stbuf->st_uid = geteuid();
		stbuf->st_gid = getegid();

		return 0;
	}

	pc = new PathCrawler(path);
	if(pc == NULL)
		return -ENOMEM;
	nq = pc->break_queries();
	
	/* no queries in this path and is the real one */
	if (nq == 0 && strncmp(path+1, REAL_DIR, strlen(REAL_DIR)-1) == 0){
		res = normal_getattr(hybfs_core, path, stbuf);
		goto out;
	}
	/* no queries and no real path, mark it as invalid */
	if(nq == 0) {
		res = -ENOENT;
		goto out;
	}
	first = pc->get_first_path();
	last = pc->get_rel_path();
	if(first.length() == 0 && last.length() == 0) {
	/* a query is treated like a virtual directory */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		res = 0;
		goto out;
		
	}
	/* if we have a path, but the real root dir is not specified, than is an error */
	DBG_PRINT("first component is %s\n", first.c_str());
	if(first.length() > 0) {
		if(!pc->is_real()) {
			DBG_SHOWFC();
			res = -ENOENT;
			goto out;
		}
		first.erase(0,strlen(REAL_DIR));
	}
	first.append(last);
	
	/* now, get a stat on the absolute path */
	p = resolve_path(hybfs_core, first.c_str(), &brid);
	if(p == NULL) {
		res = -ENOMEM;
		goto out;
	}
	DBG_PRINT(" I have real path: %s \n", p->c_str());
	res = lstat(p->c_str(), stbuf);
	if(res)
		res = -errno;
	if(res == -ENOENT) {
		/* FIXME - swallow the error? */
		hybfs_core->virtual_remove_file(first.c_str(), brid);
	}
	
	delete p;
	
out:
	delete pc;

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
