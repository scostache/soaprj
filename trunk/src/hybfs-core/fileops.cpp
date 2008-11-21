/*
 fileops.c - File operations
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "hybfs.h"
#include "misc.h"
#include "hybfsdef.h"
#include "path_crawler.hpp"

/* 
 * Warning: the rename is done properly for a SINGLE branch. 
 * How should it change if we have many directories from different
 * file systems underneath us?
 */

static int normal_rename(HybfsData *data, const char *from, const char *to, int rootlen)
{
	string *pf, *pt;
	int ret = 0;
	int brid_to, brid_from;

	DBG_SHOWFC();
	
	/* get absolute paths for both files */
	pf = resolve_path(data, from+rootlen, &brid_from);
	if (pf == NULL)
		return -ENOMEM;
	pt = resolve_path(data, to+rootlen, &brid_to);
	if (pt == NULL)
		return -ENOMEM;
	
	if(brid_to != brid_from) {
		PRINT_ERROR("Cannot do rename between multiple directories!\n");
		return -EINVAL;
	}
	/* do the real rename */
	ret = rename(pf->c_str(), pt->c_str());
	if (ret == 0) {
		ret = data->virtual_replace_query(from, to, brid_from);
	}

	delete pf;
	delete pt;

	return ret;
}

int hybfs_rename(const char *from, const char *to)
{
	int res = 0;
	int brid;
	int nqueries;
	int rootlen;
	string *abspath;
	string to_copy;
	PathCrawler *pcf = NULL; 
	PathCrawler *pct = NULL;
	
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();
	DBG_PRINT("from: %s to: %s\n", from, to);
	
	/* sanity checks */
	if(IS_ROOT(from) || IS_ROOT(to))
		return -EINVAL;
	
	/* For now accept only adding or changing tags for a real path.
	 * A real path should only come from the virtual directory 'path:/' */
	pcf = new PathCrawler(from);
	if(pcf == NULL)
		return -ENOMEM;
	
	DBG_SHOWFC();
	nqueries = pcf->break_queries();
	if(nqueries != 0) {
		res = -EINVAL;
		goto out;
	}
	DBG_SHOWFC();
	rootlen = strlen(REAL_DIR);
	if(strncmp(from+1, REAL_DIR,rootlen - 1) !=0) {
		res = -EINVAL;
		goto out;
	}
	DBG_SHOWFC();
	/* now check the second path */
	pct = new PathCrawler(to);
	if(pct == NULL) {
		res = -ENOMEM;
		goto out;
	}
	DBG_SHOWFC();
	/* If the second is a real path, then do the real rename */
	nqueries = pct->break_queries();
	if(nqueries == 0 && strncmp(to+1, REAL_DIR, rootlen-1) ==0) {
		DBG_PRINT("from %s to %s\n", from, to);
		res = normal_rename(hybfs_core, from, to, rootlen);
		goto out;
	}
	DBG_SHOWFC();
	
	/* uh, now try to add all the tag-value pairs to the DB */
	while(pct->has_next_query()) {
		string to_query = pct->pop_next_query();
		/* check again if is a path; this could happen at the beginning or the end
		skip the first slash. I should remember the path so that I'll do the rename
		after modifing the tags. */
		if(to_query.find(REAL_DIR) == 1) {
			to_copy = to_query;
		}
		
		res = hybfs_core->virtual_addtag(to_query.c_str(), from+rootlen);
		if (res)
			goto out;
		
	}
	DBG_SHOWFC();
	/* now, if I need to do a real rename, remember to do it here */
	if(to_copy.length() > 0)
		res = normal_rename(hybfs_core, from, to_copy.c_str(), rootlen);

out:
	if(pcf)
		delete pcf;
	if(pct)
		delete pct;
	
	return res;
}

int hybfs_unlink(const char *path)
{
	int res, brid;
	std::string *p;

	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();

	/* if the path is real than delete the file, else, if it's a query, get the file names
	 * from the database and delete each file */
	
	p = resolve_path(hybfs_core, path, &brid);
	res = unlink(p->c_str());
	
	delete p;
	
	if (res == -1)
	return -errno;

	return 0;
}
