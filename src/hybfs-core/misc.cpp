/*
 misc.cpp - Miscellaneous stuff
 
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


std::string * make_absolute(const char * relpath)
{
	char cwd[PATHLEN_MAX];
	size_t cwdlen;
	int trslash;
	std::string *abspath = new string(relpath);
	
	if(abspath->at(0) == '/')
		return abspath;
	
	if (!getcwd(cwd, PATHLEN_MAX)) {
		perror("Unable to get current working directory");
		delete abspath;
		
		return NULL;
	}
	cwdlen = strlen(cwd);
	ABORT((!cwdlen),"Zero-sized length of CWD!");
	
	trslash = (cwd[cwdlen-1] =='/') ? 1 : 0;
	/*append the cwd to the relative path*/
	abspath->insert(0, cwd, cwdlen);
	if(!trslash)
		abspath->insert(cwdlen,"/");
	/* must have a trailing slash */
	if(abspath->at(abspath->length()-1) != '/')
		abspath->push_back('/');
	
	DBG_PRINT("absolute branch path: %s\n", abspath->c_str());
	
	return abspath;
}

std::string * resolve_path(HybfsData *hybfs_core, const char *path, int *brid)
{
	std::string *abspath;

	/* TODO 
	 * of course, the path should be found, but in our case, we start with
	 * only one mounted directory
	 */
	*brid = 0;
	abspath = new string(hybfs_core->get_branch_path(*brid));
	abspath->append(path);

	return abspath;
}

