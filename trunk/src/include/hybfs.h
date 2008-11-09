/*
 HYBFS - semantic userspace virtual file system
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HYBFS_H
#define HYBFS_H


#include <fuse.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#include "hybfsdef.h"

/* for debugging */

#ifdef DBG

#define dbg_print(...) 					  \
	do {						  \
		printf("%s(): %d: ", __func__, __LINE__); \
		printf(__VA_ARGS__);			  \
	} while (0);
#define DBG_SHOWFC() printf("%s(): %d: \n", __func__, __LINE__);

#else

#define dbg_print(...) 
#define DBG_SHOWFC()

#endif


#define ABORT(cond,message) \
	if(cond) { fprintf(stderr, "Abort from function %s : %s\n",__func__,message); \
	exit(1); } \


/* fileops.c - File operations */

int hybfs_open(const char *path, struct fuse_file_info *fi);
int hybfs_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi);
int hybfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi);
int hybfs_rename(const char *from, const char *to);
int hybfs_unlink(const char *path);
int hybfs_release(const char *path, struct fuse_file_info *fi);
int hybfs_readlink(const char *path, char *buf, size_t size);
int hybfs_link(const char *from, const char *to);
int hybfs_symlink(const char *from, const char *to);

int hybfs_chmod(const char *path, mode_t mode);
int hybfs_chown(const char *path, uid_t uid, gid_t gid);
int hybfs_truncate(const char *path, off_t size);

/* dirops.c - Directory operations */

int hybfs_mkdir(const char *path, mode_t mode);
int hybfs_rmdir(const char *path);

/* readdir.c */

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi);

/* stats.c - Attributes and stats */

int hybfs_getattr(const char *path, struct stat *stbuf);
int hybfs_access(const char *path, int mask);
int hybfs_utimens(const char *path, const struct timespec ts[2]);
int hybfs_statfs(const char *path, struct statvfs *stbuf);

#endif /* HYBFS_H */
