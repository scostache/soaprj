/* 
 vdir_ops.c - Virtual directory operations
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <string.h>
#include <stdlib.h>
#include <fuse.h>

#include "misc.h"
#include "db_backend.h"
#include "hybfsdef.h"

/*
 * Checks if the query is valid. This means that all the tags from the
 * query exist in the DB. We also set the flags value to indicate that we have
 * a real path and/or tags.
 * The return value is negative if one of the specified tags doesn't exist.
 */
int vdir_validate(const char *path, int *flags)
{
	int res, _flags;
	char **ptr;
	char *copy, *tag;

	/* Uh, we don't have a good parser so I suppose that
	 * there is only one tag in the path */

	copy = strdup(path);
	ptr = (char **)&copy;
	
	_flags = 0;
	res = 1;
	while ((tag = strsep(ptr, "/")) != NULL) {
		if (strlen(tag) == 0)
			continue;
		
		/* now, if the tag is a real path */
		
		/*else, we have a tag, check if it really exists */
		_flags |= HAS_TAG;
		
		/* Double uh: here it should be a logic operation with
		 * res, extracted from the query but, again, no parser...*/
		res &= db_check_tag(tag);
	}
	
	free(copy);
	*flags =  _flags;
	
	return res;
}

/*Adds file info coresponding to this file, to the db.*/
int vdir_add_tag(char *tag, char *path)
{
	char abspath[PATHLEN_MAX];
	struct stat stbuf;
	file_info_t *finfo;
	int len, brid;
	int res;
	
	/*make this path absolute*/
	resolve_path(path, abspath,&brid, PATHLEN_MAX);
	
	memset(&stbuf,0, sizeof(struct stat));
	len = strlen(abspath);
	
	res = lstat(abspath, &stbuf);
	if(res)
		return -1;

	finfo = (file_info_t*) malloc(sizeof(file_info_t)+len+1);
	finfo->brid = brid;
	finfo->fid = stbuf.st_ino;
	finfo->mode = stbuf.st_mode;
	finfo->namelen = len;
	memcpy(&finfo->name[0],abspath,len);
	
	res = db_add_file_info(tag, finfo);
	
	free(finfo);
	
	return res;
}
