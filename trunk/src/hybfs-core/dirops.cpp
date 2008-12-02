/*
 dirops.c - Dir operations
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <unistd.h>
#include "hybfs.h"

int hybfs_mkdir(const char *path, mode_t mode)
{
	int res;

	DBG_SHOWFC();

	res = mkdir(path, mode);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_rmdir(const char *path)
{
	int res;

	DBG_SHOWFC();

	/* if it is a real path only, then delete the dir, and delete the
	 * file info from the db - wait, should I have any left? */
	res = rmdir(path);
	if (res) {
		res = -errno;
		goto out;
	}
	
	/* if it's a dir and a query? */
	
	/* and if's only a query? should I check if it returns something? */
	res = -ENOTEMPTY;
out:

	return 0;
}
