/*
 readdir.c - File listing
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 */

#include <string.h>

#include "hybfs.h"

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
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
