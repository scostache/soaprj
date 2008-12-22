/* 
base_module.cpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "base_module.hpp"
#include "core/virtualdir.hpp"

using namespace hybfs;

GenericModule::GenericModule()
{
	vdir = NULL;
	path = NULL;
}

GenericModule::~GenericModule()
{
	if (vdir != NULL)
		delete vdir;
	if (path != NULL)
		delete path;
}

void GenericModule::init_vdir(const char * path)
{
	this->path = strdup(path);
	vdir = new VirtualDirectory(path);
	vdir->init();
}

file_info_t * GenericModule::get_file_info(const char * path)
{
	struct stat *buf;			// file information structure for stat function
	file_info_t *finfo = NULL;	// structure with info to populate the database

	buf = (struct stat *) malloc (sizeof(struct stat));
	if (!buf)
		return NULL;

	/* if the stat function returns an error */
	if (0 != stat(path, buf)) {
		return NULL;
	}

	finfo = (file_info_t *) malloc (sizeof(file_info_t) + strlen(path) + 1 - strlen(this->path));
	if (!finfo) {
		free (buf);
		return NULL;
	}

	/* copy stat information in the finfo structure */
	finfo->fid = buf->st_ino;
	finfo->mode = buf->st_mode;
	finfo->namelen = strlen(path) - strlen(this->path);
	strcpy(finfo->name, path + strlen(this->path));

	/* clean up the mess */
	delete (buf);

	return finfo;
}
