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

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include "hybfs.h"
#include "hybfsdef.h"
#include "misc.h"
#include "db_backend.h"

hybfs_t hybfs_core;

static struct fuse_opt options[] = {
	FUSE_OPT_KEY("--help", KEY_HELP),
	FUSE_OPT_KEY("-h", KEY_HELP),
	FUSE_OPT_END
};

#define ADD_FUSE_OPT(p) \
	if (fuse_opt_add_arg(&args, p)) { \
		fprintf(stderr, "Can't add %s to the list of options, aborting!\n",p); \
		exit(1); \
	}

static void print_usage()
{
	fprintf(stderr,
	"HybFS\n"
	"Usage: hybfs directory mountpoint\n"
	"general options: (you don't have any choice here)\n"
	"    -h   --help            print help\n");
}

int hybfs_opts(void *data, const char *arg, int key,
                struct fuse_args *outargs)
{
	(void)data;

	int res = 0;

	switch (key)
	{
	case FUSE_OPT_KEY_NONOPT:
		res = parse_branches(arg);
		if (res > 0)
			return 0;
		hybfs_core.retval = 1;
		return 1;
	case KEY_HELP:
		print_usage();
		fuse_opt_add_arg(outargs, "-ho");
		hybfs_core.doexit = 1;
		return 0;
	default:
		hybfs_core.retval = 1;
		return 1;
	}
}


static struct fuse_operations hybfs_oper = { 
		.getattr = hybfs_getattr, 
		.access = hybfs_access,
                .readlink = hybfs_readlink, 
                .readdir = hybfs_readtagdir,
//               .mkdir = hybfs_mkdir,
//               .symlink = hybfs_symlink,
                .unlink = hybfs_unlink,
//               .rmdir = hybfs_rmdir,
                .rename = hybfs_rename,
//               .link = hybfs_link,
//               .chmod = hybfs_chmod,
//               .chown = hybfs_chown,
//                .truncate = hybfs_truncate,
//                .utimens = hybfs_utimens,
//                .create = hybfs_create,
                .open = hybfs_open,
                .read = hybfs_read,
                .write = hybfs_write,
                .release = hybfs_release,
                .statfs = hybfs_statfs, 
#ifdef HAVE_SETXATTR

// extended attributes support

#endif /* HAVE_SETXATTR */
};


int main(int argc, char *argv[])
{
	int i;
	int res;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

#ifdef DBG
	for(i=0; i<argc; i++)
	printf("%s : ", argv[i]);
	printf("hybfs: end argument list\n");
#endif

	hybfs_core.nbranches = 0;
	hybfs_core.doexit = 0;
	hybfs_core.retval = 0;
	
	if (fuse_opt_parse(&args, NULL, options, hybfs_opts) == -1)
		return 1;

	/* set additional options for fuse */
	if (getuid() == 0 || getgid() == 0) {
		ADD_FUSE_OPT("-odefault_permissions");
	}
	ADD_FUSE_OPT("-ononempty");
	
	res = db_init_storage();
	if(res != 0)
		return 1;
	
	umask(0);
	res = fuse_main(args.argc, args.argv, &hybfs_oper, NULL);
	
	db_close_storage();
	
	return hybfs_core.doexit ? hybfs_core.retval : res;
}
