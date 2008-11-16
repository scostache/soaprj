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
#include "misc.h"
#include "hybfsdef.h"


int hybfs_rename(const char *from, const char *to)
{
	int res = 0;

	DBG_SHOWFC();
	DBG_PRINT("from: %s to: %s\n", from, to);

	/* Here we need to parse and validate both paths */

	/*Ok, I'm  lazy and I don't have a parser, so I'll just
	 * take the first argument as the path and the second 
	 * as the tag */
	//res = vdir_add_tag((char *)to, (char *)from);

	return res;
}

int hybfs_unlink(const char *path)
{
	int res, brid;
	std::string *p;

	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();

	p = resolve_path(hybfs_core, path, &brid);
	res = unlink(p->c_str());
	
	delete p;
	
	if (res == -1)
	return -errno;

	return 0;
}
