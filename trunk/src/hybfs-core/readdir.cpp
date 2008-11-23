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
                            fuse_fill_dir_t filler)
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

	ret = hybfs_core->virtual_readdir("/", buf, filler);

	return ret;
}

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
	const char * brpath;
	int ret;
	int i, n, brid;
	unsigned int path_len;
	std::string *p;
	HybfsData *hybfs_core = get_data();

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	if (strcmp(path, "/") == 0)
		return fill_root(hybfs_core, buf, filler);

	DBG_PRINT("my path is #%s#\n", path);
	/* is this the real path ? */
	
	/* call readdir for each branch directory */
	path_len = strlen(REAL_DIR);
	if (strncmp(path+1, REAL_DIR, path_len-1) == 0) {
		/* the root path */
		if (strlen(path+1) == path_len-1) {
			n = hybfs_core->get_nbranches();
			for (i=0; i<n; i++) {
				brpath = hybfs_core->get_branch_path(i);
				ret = normal_readdir(brpath, buf, filler);
				if (ret)
					break;
			}
			return ret;
		}
		/* the sub-directory */
		p = resolve_path(hybfs_core, path+1+strlen(REAL_DIR), &brid);
		if (p==NULL)
			return -ENOMEM;

		ret = normal_readdir(p->c_str(), buf, filler);
		delete p;

		return ret;
	}

	/* call the virtual directory readdir */
	ret = hybfs_core->virtual_readdir(path, buf, filler);

	return ret;
}

