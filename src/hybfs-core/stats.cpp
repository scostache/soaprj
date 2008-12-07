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
#include "path_data.hpp"


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
	std::string first, last;
	PathCrawler *pc= NULL;
	PathData    *pd= NULL;

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
	
	/* if we have a path, but the real root dir is not specified,
	 *  than is an error */
	first = pc->get_first_path();
	DBG_PRINT("first component is %s\n", first.c_str());
	if(first.length() > 0 && !pc->is_real()) {
		DBG_SHOWFC();
		res = -ENOENT;
		goto out;
	}
		
	pd = new PathData(path, hybfs_core, pc);
	if(pd == NULL) {
		res = -ENOMEM;
		goto out;
	}
		
	if(pd->check_path_data() == 0) {
	/* a query is treated like a virtual directory */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		res = 0;
		goto out;
		
	}
	
	DBG_PRINT(" I have real path: %s \n", pd->abspath_str());
	res = lstat(pd->abspath_str(), stbuf);
	if(res)
		res = -errno;
	if(res == -ENOENT) {
		/* FIXME - swallow the error? */
		hybfs_core->virtual_remove_file(pd->relpath_str(), brid);
	}
	
out:
	delete pc;
	if(pd)
		delete pd;
	
	return res;
}


int hybfs_access(const char *path, int mask)
{
	int res;
	PathCrawler *pc;
	PathData    *pd;
	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();
	
	if (strcmp(path, "/") == 0 || IS_ROOT(path+1)) {
		DBG_PRINT("Access ROOT PATH\n");
		return 0;
	}
	pc = new PathCrawler(path);
	if(pc == NULL)
		return -ENOMEM;
	
	pc->break_queries();
	
	pd = new PathData(path, hybfs_core, pc);
	if(pd == NULL) {
		delete pc;
		return -ENOMEM;
	}
	
	if(pd->check_path_data() == 0) {
		res = 0;
		goto out;
	}
	
	res = access(pd->abspath_str(), mask);
	if (res == -1)
		res = -errno;

out:
	delete pc;
	delete pd;
	
	return res;
}
