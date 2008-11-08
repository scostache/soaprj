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

/*
 * Makes an absolute path from a relative one. The string returned must be freed.
 */
char *make_absolute(char *relpath)
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

	abspath = malloc(abslen);
	ABORT((abspath == NULL), "malloc failed");
	sprintf(abspath, "%s/%s/", cwd, relpath);

	return abspath;
}

/* 
 * Adds one branch to the list. The space is already allocated 
 */
void add_branch(char *branch)
{
	struct stat buf;
	
	dbg_print("adding branch #%s# \n", branch);

	if (lstat(branch, &buf) == -1) {
		fprintf(stderr, "hybfs: Warning! This branch is not valid!");
		return;
	}
	if (!S_ISDIR(buf.st_mode)) {
		fprintf(stderr, "hybfs: Warning! This branch is not a directory!");
		return;
	}
	
	hybfs_core.branches[hybfs_core.nbranches].path = make_absolute(branch);
	hybfs_core.nbranches ++;
}

/*
 *  Extracts the branches from the options string 
 */ 
int parse_branches(const char *arg)
{
	char *buf, *branch;
	char **ptr;
	int i, branches;

	if (hybfs_core.nbranches)
		return 0;

	ABORT((arg[0] == '\0'), "HybFS: No branches specified! \n");

	branches = 1;
	for (i = 0; arg[i]; i++)
		if (arg[i] == ':')
			branches++;

	hybfs_core.branches = malloc(branches*sizeof(hybfs_branch_t));
	ABORT((hybfs_core.branches == NULL), "malloc failed");

	/* parse the options. This follows the model: "dir1:dir2:dir3" */
	buf = strdup(arg);
	ptr = (char **)&buf;
	while ((branch = strsep(ptr, ROOT_SEP)) != NULL) {
		if (strlen(branch) == 0)
			continue;
		add_branch(branch);
	}

	free(buf);
	
	return hybfs_core.nbranches;
}

/* 
 * This should resolve the file path, if we have multiple directories/branches
 */
void resolve_path(const char *path,char *abspath, int total_size)
{
	/* TODO 
	 * of course, the path should be found, but in our case, we start with
	 * only one mounted directory
	 */
	snprintf(abspath, total_size, "%s%s", hybfs_core.branches[0].path, path);
}

