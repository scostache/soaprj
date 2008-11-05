/*
 	dirops.c - Dir operations
 	
 	Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei
*/

#include <unistd.h>
#include "hybfs.h"

int hybfs_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

int hybfs_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}
