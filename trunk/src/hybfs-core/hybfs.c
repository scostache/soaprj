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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "hybfs.h"
#include "readdir.h"
#include "stats.h"

enum {	KEY_HELP, KEY_VERSION};

int unionfs_opt_proc(void *data, const char *arg, int key,
                struct fuse_args *outargs)
{
	(void)data;

	int res = 0; // for general purposes

	switch (key)
	{
	case FUSE_OPT_KEY_NONOPT:
		res = parse_branches(arg);
		if (res > 0)
			return 0;
		uopt.retval = 1;
		return 1;
	case KEY_HELP:
		print_help(outargs->argv[0]);
		fuse_opt_add_arg(outargs, "-ho");
		uopt.doexit = 1;
		return 0;
	default:
		uopt.retval = 1;
		return 1;
	}
}

static struct fuse_operations hybfs_oper = { 
		.getattr = hybfs_getattr, 
		.access = hybfs_access,
                .readlink = hybfs_readlink, 
                .readdir = hybfs_readdir,
                .mkdir = hybfs_mkdir,
                .symlink = hybfs_symlink,
                .unlink = hybfs_unlink,
                .rmdir = hybfs_rmdir,
                .rename = hybfs_rename,
                .link = hybfs_link,
                .chmod = hybfs_chmod,
                .chown = hybfs_chown,
                .truncate = hybfs_truncate,
                .utimens = hybfs_utimens,
                .open = hybfs_open,
                .read = hybfs_read,
                .write = hybfs_write,
                .statfs = hybfs_statfs, 
#ifdef HAVE_SETXATTR

// extended attributes support

#endif /* HAVE_SETXATTR */
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &hybfs_oper, NULL);
}
