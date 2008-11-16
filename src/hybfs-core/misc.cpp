/*
 misc.c - Miscellaneous stuff
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "hybfs.h"
#include "hybfsdef.h"
#include "misc.h"

/*
 * Makes an absolute path from a relative one. The returned string
 * must be freed.
 */
char * make_absolute(char *relpath)
{
	int len, abslen;
	char *abspath;
	char cwd[PATHLEN_MAX];
	size_t cwdlen;

	/* check for absolute path */
	if (*relpath == '/')
		return relpath;

	/* check for trailing slash */
	abslen = 0;
	len = strlen(relpath);
	if (relpath[len - 1] == '/')
		abslen = 1;

	if (!getcwd(cwd, PATHLEN_MAX)) {
		perror("Unable to get current working directory");
		return NULL;
	}

	cwdlen = strlen(cwd);
	ABORT((!cwdlen),"Zero-sized length of CWD!");

	abslen += cwdlen + strlen(relpath) + 2;

	abspath = (char*)malloc(abslen);
	ABORT((abspath == NULL), "malloc failed");
	sprintf(abspath, "%s/%s/", cwd, relpath);

	return abspath;
}

/* 
 * This should resolve the file path, if we have multiple directories/branches.
 */
std::string * resolve_path(HybfsData *hybfs_core, const char *path, int *brid)
{
	std::string *abspath;

	/* TODO 
	 * of course, the path should be found, but in our case, we start with
	 * only one mounted directory
	 */
	*brid = 0;
	abspath = new string(hybfs_core->get_branch_path(*brid));
	*abspath += path;

	return abspath;
}

/*
 * Concatenates two paths in one. Both paths must be valid (not NULL).
 * The last must be a relative path. 
 * The result must be freed afterwards.
 */
char * concat_paths(const char *src1, const char *src2, int isdir)
{
	char * dest;
	int len1, len2, trslash;

	trslash = 0;
	len1 = strlen(src1);
	len2 = strlen(src2);

	if (len1 == 0 || len2 == 0) {
		fprintf(stderr, "%s : One path is NULL!\n", __func__);
		return NULL;
	}
	if (src1[0] !='/' || src2[0] == '/') {
		fprintf(stderr, "%s : Invalid argument(s)!\n", __func__);
		return NULL;
	}

	if (src1[len1-1] != '/')
		trslash++;
	if (src2[len2-1] != '/' && isdir)
		trslash++;

	dest = (char*)malloc(len1+len2+trslash+1);
	memcpy(dest, src1, len1);

	if (src1[len1-1] != '/') {
		dest[len1] = '/';
		len1++;
	}
	memcpy(dest+len1, src2, len2);
	if (src2[len2-1] != '/' && isdir) {
		dest[len1+len2] = '/';
		len2++;
	}
	dest[len1+len2] = '\0';

	return dest;
}
