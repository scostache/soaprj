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

int hybfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char p[PATHLEN_MAX];
	
	DBG_SHOWFC();
	
	resolve_path(path,p,PATHLEN_MAX);

	res = open(p, fi->flags);
	if (res == -1)
	return -errno;

	/* for future use */
	fi->fh = (unsigned long) res;
	
	return 0;
}

int hybfs_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi)
{
	int res;

	(void) fi;

	DBG_SHOWFC();

	res = pread(fi->fh, buf, size, offset);
	if (res == -1)
	res = -errno;

	return res;
}

int hybfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi)
{
	int res;

	(void) fi;

	DBG_SHOWFC();

	res = pwrite(fi->fh, buf, size, offset);
	if (res == -1)
	res = -errno;

	return res;
}

int hybfs_rename(const char *from, const char *to)
{
	int res;
	char f[PATHLEN_MAX], t[PATHLEN_MAX];

	DBG_SHOWFC();

	resolve_path(from,f,PATHLEN_MAX);
	resolve_path(to,t,PATHLEN_MAX);
	res = rename(f, t);
	if (res == -1)
		return -errno;

	return 0;
}

int hybfs_unlink(const char *path)
{
	int res;
	char p[PATHLEN_MAX];

	DBG_SHOWFC();

	resolve_path(path,p,PATHLEN_MAX);
	res = unlink(p);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_release(const char *path, struct fuse_file_info *fi)
{
	int res;
	
	DBG_SHOWFC();
	
	res = close(fi->fh);
	if (res == -1) return -errno;

	return 0;
}

int hybfs_readlink(const char *path, char *buf, size_t size)
{
	int res;
	char p[PATHLEN_MAX];

	DBG_SHOWFC();

	resolve_path(path,p,PATHLEN_MAX);
	res = readlink(path, buf, size - 1);
	if (res == -1)
	return -errno;

	buf[res] = '\0';
	return 0;
}

int hybfs_symlink(const char *from, const char *to)
{
	int res;

	DBG_SHOWFC();

	res = symlink(from, to);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_link(const char *from, const char *to)
{
	int res;

	DBG_SHOWFC();

	res = link(from, to);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_chmod(const char *path, mode_t mode)
{
	int res;

	DBG_SHOWFC();

	res = chmod(path, mode);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	DBG_SHOWFC();

	res = lchown(path, uid, gid);
	if (res == -1)
	return -errno;

	return 0;
}

int hybfs_truncate(const char *path, off_t size)
{
	int res;

	DBG_SHOWFC ();

	res = truncate(path, size);
	if (res == -1)
	return -errno;

	return 0;
}
