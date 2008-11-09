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

	(void) offset;
	(void) fi;
	
	char p[PATHLEN_MAX];
	
	DBG_SHOWFC();

	resolve_path(path,p,PATHLEN_MAX);
	dp = opendir(p);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}


int hybfs_readtagdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi)
{
	DBC *pcursor;
	file_info_t finfo;
	int ret;

	(void) offset;
	(void) fi;
	
	DBG_SHOWFC();

	/* open a cursor to the main table */
	ret = hybfs_db.main_table->cursor(hybfs_db.main_table, NULL, &pcursor, 0);
	if(ret)
		return -EIO;
	
	memset(&finfo, 0, sizeof(finfo));
	ret = db_get_file_info(path, pcursor, 1, &finfo);
	while (ret != DB_NOTFOUND)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		
		/*fill the info about this particular file*/
		st.st_ino = finfo.fid;
		st.st_mode = finfo.mode;
		if (filler(buf, finfo.name, &st, 0))
			break;
		
		db_get_file_info((char*)path, pcursor, 0, &finfo);
	}

	/* close the used cursor */
	pcursor->close(pcursor);
	
	/* C'est fini! */
	return 0;
}
