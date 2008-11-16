/*
 readdir.c - File listing
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <string.h>

#include "hybfs.h"
#include "misc.h"
#include "db_backend.h"

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;
	int brid;
	
	std::string *p;
	HybfsData *hybfs_core = get_data();
	
	(void) offset;
	(void) fi;
	
	DBG_SHOWFC();
	
	p = resolve_path(hybfs_core, path, &brid);
	
	DBG_PRINT("path: %s\n",path);
	
	dp = opendir(p->c_str());
	if (dp == NULL) {
		delete path;
		
		return -errno;
	}
	while ((de = readdir(dp)) != NULL)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}
	delete p;
	
	closedir(dp);
	return 0;
}
