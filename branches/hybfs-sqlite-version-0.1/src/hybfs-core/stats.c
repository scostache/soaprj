/*
 stats.c - file attributes and stats
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "hybfs.h"
#include "misc.h"

int hybfs_getattr(const char *path, struct stat *stbuf)
{
	int res, flags;

	DBG_SHOWFC();
	DBG_PRINT("path: %s\n",path);
	
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		
		return 0;
	}
	/* this is used for lookup, so check if
	 * the query is valid */
	res = vdir_validate(path, &flags);
	if(res && (!(flags & HAS_PATH)))
		return 0;

	return -EIO;
}

int hybfs_access(const char *path, int mask)
{
	int res;
	char p[PATHLEN_MAX];
	int flags, brid;

	DBG_SHOWFC();
	
	if (strcmp(path, "/") == 0) {
		return 0;
	}

	/* this is to verify if the query expressed by path is valid */
	res = vdir_validate(path, &flags);
	if(res && (!(flags & HAS_PATH)))
		return 0;
	
	if (flags & HAS_PATH) {
		/* if it's a normal path, not a query, then verify the real source*/
		resolve_path(path, p,&brid, PATHLEN_MAX);
		res = access(p, mask);
	}
	if (res == -1)
		return -errno;

	return 0;
}

int hybfs_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

	DBG_SHOWFC();

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_statfs(const char *path, struct statvfs *stbuf)
{
	int res, brid;
	char p[PATHLEN_MAX];

	DBG_SHOWFC();

	resolve_path(path,p,&brid,PATHLEN_MAX);
	res = statvfs(p, stbuf);
	if (res == -1)
	return -errno;

	return 0;
}
