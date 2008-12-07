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
#include "hybfs_data.hpp"

/* for debugging */

#ifdef DBG

#define DBG_PRINT(...) 					  \
	do {						  \
		printf("%s(): %d: ", __func__, __LINE__); \
		printf(__VA_ARGS__);			  \
	} while (0);
#define DBG_SHOWFC() printf("%s(): %d: \n", __func__, __LINE__);

#else

#define DBG_PRINT(...) 
#define DBG_SHOWFC()

#endif

#define PRINT_ERROR(...) fprintf(stderr, __VA_ARGS__);

#define ABORT(cond,message) \
	if(cond) { fprintf(stderr, "Abort from function %s : %s\n",__func__,message); \
	exit(1); }

#define IS_ROOT(path) \
	((strcmp(path, REAL_DIR) == 0 || (strncmp(path, REAL_DIR, strlen(REAL_DIR) -1) == 0 \
		&& strlen(path) == strlen(REAL_DIR) -1)) ? 1 : 0)

/* hybfs.cpp - common operations */

inline HybfsData *get_data()
{
	return (HybfsData *) fuse_get_context()->private_data;
}

/* fileops.cpp - File operations */

int hybfs_rename(const char *from, const char *to);

int hybfs_open(const char *path, struct fuse_file_info *fi);
int hybfs_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi);
int hybfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi);
int hybfs_flush(const char *path, struct fuse_file_info *fi);
int hybfs_truncate(const char *path, off_t size);
int hybfs_release(const char *path, struct fuse_file_info *fi);

int hybfs_mknod(const char *path, mode_t mode, dev_t rdev);
int hybfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int hybfs_unlink(const char *path);

int hybfs_utimens(const char *path, const struct timespec ts[2]);
int hybfs_chmod(const char *path, mode_t mode);
int hybfs_chown(const char *path, uid_t uid, gid_t gid);

/* dirops.cpp - Directory operations */

int hybfs_mkdir(const char *path, mode_t mode);
int hybfs_rmdir(const char *path);

/* readdir.cpp */

int hybfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi);

/* stats.cpp - Attributes and stats */

int hybfs_getattr(const char *path, struct stat *stbuf);
int hybfs_access(const char *path, int mask);

#endif /* HYBFS_H */
