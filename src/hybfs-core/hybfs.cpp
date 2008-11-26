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



#define ADD_FUSE_OPT(p) \
	if (fuse_opt_add_arg(&args, p)) { \
		fprintf(stderr, "Can't add %s to the list of options, aborting!\n",p); \
		return 1; \
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
	HybfsData *hybfs_core = (HybfsData *) data;
	
	int res = 0;
	
	DBG_PRINT("arguments: %s\n", arg);

	switch (key)
	{
	case FUSE_OPT_KEY_NONOPT:
		res = hybfs_core->parse_branches(arg);
		if (res > 0)
			return 0;
		hybfs_core->retval = 1;
		return 1;
	case KEY_HELP:
		print_usage();
		fuse_opt_add_arg(outargs, "-ho");
		hybfs_core->doexit = 1;
		return 0;
	default:
		hybfs_core->retval = 1;
		return 1;
	}
}

#define INIT_KEY(index, string, key) \
	options[index].templ = string; \
	options[index].offset = -1U; \
	options[index].value = key; \

int main(int argc, char *argv[])
{
	int i;
	int res, exit, retval;
	struct fuse_args args;
	static struct fuse_opt options[3];
	static struct fuse_operations hybfs_oper;
	
	HybfsData *data = new HybfsData(NULL);
	
	args.argc = argc;
	args.argv = argv;
	args.allocated = 0;
	
	/* -------FUSE FS operations------- */
	hybfs_oper.getattr = hybfs_getattr;
	hybfs_oper.access  = hybfs_access;
	hybfs_oper.readdir = hybfs_readdir;
	hybfs_oper.unlink  = hybfs_unlink;
	hybfs_oper.rename  = hybfs_rename;
	hybfs_oper.open    = hybfs_open;
	hybfs_oper.read    = hybfs_read;
	hybfs_oper.write   = hybfs_write;
	hybfs_oper.release = hybfs_release;
	hybfs_oper.create  = hybfs_create;
	hybfs_oper.mknod   = hybfs_mknod;
	/* ------end FUSE interface------ */
	
	INIT_KEY(0,"--help", KEY_HELP);
	INIT_KEY(1,"-h", KEY_HELP);
	INIT_KEY(2,NULL,0);

#ifdef DBG
	for(i=0; i<argc; i++)
	printf("%s : ", argv[i]);
	printf("hybfs: end argument list\n");
#endif

	if (fuse_opt_parse(&args, data, options, hybfs_opts) == -1) {
		
		delete data;
		return 1;
	}
	/* set additional options for fuse */
	if (getuid() == 0 || getgid() == 0) {
		ADD_FUSE_OPT("-odefault_permissions");
	}
	ADD_FUSE_OPT("-ononempty");

	res = data->start_db_storage();
	if(res != 0) {
		delete data;
		return 1;
	}
	
	umask(0);
	/* pass the data to each context from now on */
	res = fuse_main(args.argc, args.argv, &hybfs_oper, data);
	
	retval = data->retval;
	exit = data->doexit;
	
	if(data)
		delete data;
	
	return exit ? retval : res;
}
