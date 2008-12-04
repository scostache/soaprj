/*
 readdir.c - File listing
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <string.h>

#include "hybfs.h"
#include "misc.h"
#include "db_backend.hpp" /* for METADIR */

static inline int normal_readdir(const char *path, void *buf,
                                 fuse_fill_dir_t filler)
{
	DIR *dp;
	struct dirent *de;
	struct stat st;

	DBG_PRINT("path = %s\n", path);

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..")
		                == 0)
			continue;
		if(strcmp(de->d_name, METADIR) == 0)
			continue;

		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);

	return 0;
}

static inline int fill_root(HybfsData *hybfs_core, void *buf,
                            filler_t filler)
{
	struct stat st;
	int ret = 0;

	/* we are in root: add a path to the real directories (the special
	 * tag "path" with ":" appended, and list all the tags */
	memset(&st, 0, sizeof(st));

	st.st_mode = S_IFDIR | 0755;
	st.st_nlink = 2 + hybfs_core->get_nlinks();
	st.st_uid = geteuid();
	st.st_gid = getegid();
	ret = filler(buf, REAL_DIR, &st, 0);

	ret = hybfs_core->virtual_readroot("/", buf, filler);

	return ret;
}

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
	const char * brpath;
	int ret;
	int i, n, brid, nqueries;
	unsigned int path_len;
	std::string *p = NULL;
	std::string *relpath = NULL;
	PathCrawler *pc = NULL;
	HybfsData *hybfs_core = get_data();

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	DBG_PRINT("my path is #%s#\n", path);
	if (strcmp(path, "/") == 0)
			return fill_root(hybfs_core, buf, filler);
	
	/* call readdir for each branch directory, if any */
	path_len = strlen(REAL_DIR);
	if(IS_ROOT(path+1)) {
		/* the root path */
		n = hybfs_core->get_nbranches();
		for (i=0; i<n; i++) {
			brpath = hybfs_core->get_branch_path(i);
			ret = normal_readdir(brpath, buf, filler);
			if (ret)
				return ret;
			/* list the tags and tag-value pairs for the root dir */
		}
		DBG_PRINT("Sunt in root path!\n");
		ret = hybfs_core->virtual_readroot("/", buf, filler);
		return ret;
	}
	
	/* the rest is for sub-directories and queries */
	pc = new PathCrawler(path);
	if(pc == NULL)
		return -ENOMEM;
	nqueries = pc->break_queries();
	
	if(nqueries == 0 && strncmp(path+1, REAL_DIR, strlen(REAL_DIR)-1) == 0) {
		/* if it's only a real path, do a normal readdir first */
		p = resolve_path(hybfs_core, path+strlen(REAL_DIR), &brid);
		if (p==NULL) {
			ret = -ENOMEM;
			goto out;
		}
		ret = normal_readdir(p->c_str(), buf, filler);
		/* something is wrong or the buffer is full */
		if(ret)
			goto out;
		ret = hybfs_core->virtual_readroot(path+strlen(REAL_DIR),
				buf, filler);
		goto out;
	 }
	
	/* call the virtual directory readdir */
	ret = hybfs_core->virtual_readdir(path, buf, filler);

out:
	if(p)
		delete p;
	if(relpath)
		delete relpath;
	if(pc)
		delete pc;
	
	return ret;
}

