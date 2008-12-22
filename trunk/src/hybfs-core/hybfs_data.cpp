/* 
 hybfs_data.cpp - The main class used to store important HybFs data
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "hybfs.h"
#include "core/hybfsdef.h"
#include "core/misc.h"
#include "core/hybfs_data.hpp"
#include "core/path_crawler.hpp"

namespace hybfs {

HybfsData::HybfsData(char *_mountp)
{
	mountp = _mountp;
	doexit = 0;
	retval = 0;
}

int HybfsData::add_branch(const char * branch)
{
	struct stat buf;
	string *abspath = NULL;
	VirtualDirectory *vdir;
	int ret;

	abspath = make_absolute(branch);
	if(abspath == NULL) {
		return -1;
	}
	
	if (lstat(abspath->c_str(), &buf) == -1) {
		PRINT_ERROR("hybfs: Warning! This branch is not valid!");
		ret = -1;
		goto out;
	}
	if (!S_ISDIR(buf.st_mode)) {
		PRINT_ERROR("hybfs: Warning! This branch is not a directory!");
		ret = -1;
		goto out;
	}

	branches.push_back(*abspath);

	vdir = new VirtualDirectory(abspath->c_str());
	if(vdir == NULL) {
		ret = -1;
		goto out;
	}
	if(vdir->check_for_init()) {
		ret = -1;
		goto out;
	}

	vdirs.push_back(vdir);
	ret = 0;
out:
	if(abspath != NULL)
		delete abspath;
	
	return ret;
}

int HybfsData::delete_branch(const char * branch)
{
	int i, ret;

	ret = 0;
	for (i=0; i< (int) branches.size(); i++) {
		if (strncmp(branches[i].c_str(), branch, branches[i].size() )
		                == 0) {
			/* delete the branch path */
			branches.erase(branches.begin()+i, branches.begin()+1+i);
			/* delete the associated vdir handle*/
			vdirs.erase(vdirs.begin()+i, vdirs.begin()+1+i);
			
			return 0;
		}
	}
	PRINT_ERROR("Could not find branch %s \n", branch);

	return 1;
}

/*
 *  Extracts the branches from the options string. 
 */
int HybfsData::parse_branches(const char *arg)
{
	PathCrawler pc(arg, ROOT_SEP);
	
	if (branches.size() != 0)
		return 0;

	ABORT((arg[0] == '\0'), "HybFS: No branches specified! \n");

	/* parse the options. This follows the model: "dir1:dir2:dir3" */
	while (pc.has_next()) {
		string next = pc.get_next();
		const char * branch = next.c_str();
		if (strlen(branch) == 0)
			continue;
		add_branch(branch);
	}

	return branches.size();
}

const char * HybfsData::get_branch_path(int brid)
{
	if (brid < 0 || brid > (int) branches.size())
		return NULL;

	return branches[brid].c_str();
}

int HybfsData::get_nlinks()
{
	int i, nlinks;
	int size;
	struct stat st;
	int ret;
	
	memset(&st, 0, sizeof(st));
	
	size = branches.size();
	nlinks = 0;
	
	for(i = 0; i< size; i++) {
		ret = lstat(branches[i].c_str(), &st);
		if(!ret)
			nlinks += st.st_nlink;
	}
	
	return nlinks;
}

int HybfsData::virtual_readroot(const char *path, void *buf,
		                                filler_t filler)
{
	int i, size, brid;
	int ret = 0;
	std::string *rpath;

	size = vdirs.size();
	/* call a virtual readdir for each branch */
	if (path[0] == '\0' || strcmp(path, "/") == 0) {
		for (i=0; i<size; i++) {
			ret = vdirs[i]->vdir_list_root(NULL,buf, filler);
			if (ret)
				break;
		}
		return ret;
	}

	/* find out in which branch the directory is */
	rpath = resolve_path(this, path, &brid);
	delete rpath;
	/* call virtual readdir for that branch */
	ret = vdirs[brid]->vdir_list_root(path, buf, filler);

	return ret;
}

int HybfsData::virtual_readdir(const char *query, void *buf, filler_t filler)
{
	int i, size;
	int ret = 0;
	
	if(query == NULL)
		return -EINVAL;
	
	size = vdirs.size();
	for(i=0; i<size; i++) {
		ret = vdirs[i]->vdir_readdir(query, buf, filler);
		if(ret)
			break;
	}

	return ret;
}

int HybfsData::virtual_remove_file(const char *path, int brid)
{
	int ret = 0;
	
	if(brid <0 || brid >= (int) vdirs.size() || path == NULL)
		return -EINVAL;
	
	ret = vdirs[brid]->vdir_remove_file(path);
	
	return ret;
}

int HybfsData::virtual_updatetags(PathCrawler *from, const char *path,
                                  const char *abspath, int brid)
{
	int ret = 0;
	int pathlen;
	struct stat st;
	file_info_t *finfo = NULL;
	
	if(brid <0 || brid >= (int) vdirs.size() || path == NULL)
		return -EINVAL;
	
	ret = stat(abspath, &st);
	if(ret) {
		return -errno;
	}
	
	pathlen = strlen(path);
	finfo = (file_info_t *) malloc(sizeof(file_info_t) + pathlen);
	if(finfo == NULL) {
		return -ENOMEM;
	}

	finfo->namelen = pathlen;
	memcpy(&finfo->name[0], path+1, pathlen-1);
	finfo->name[pathlen-1] = '\0';
	finfo->fid = st.st_ino;
	finfo->mode = st.st_mode;
		
	ret = vdirs[brid]->vdir_update_tags(from, finfo);
	
	free(finfo);
		
	return ret;
}

int HybfsData::virtual_addtag(PathCrawler *pc, const char *path,
                              const char *abspath, int brid)
{
	struct stat st;
	int pathlen;
	int ret = 0;
	file_info_t *finfo = NULL;
			
	DBG_PRINT("absolute path is %s\n", abspath);
	
	if(brid <0 || brid >= (int) vdirs.size() || path == NULL)
		return -EINVAL;
	if(pc->get_nqueries() == 0)
		return 0;
	
	ret = stat(abspath, &st);
	if(ret) {
		return -errno;
	}
	
	pathlen = strlen(path);
	finfo = (file_info_t *) malloc(sizeof(file_info_t) + pathlen);
	if(finfo == NULL) {
		return -ENOMEM;
	}

	finfo->namelen = pathlen;
	memcpy(&finfo->name[0], path+1, pathlen-1);
	finfo->name[pathlen-1] = '\0';
	finfo->fid = st.st_ino;
	finfo->mode = st.st_mode;
	
	ret = vdirs[brid]->vdir_add_tag(pc, finfo);
	
	free(finfo);
	
	return ret;
}

int HybfsData::virtual_replace_query(const char*relfrom, const char *relto,
                                     PathCrawler *from, PathCrawler *to, int do_fsmv)
{
	int ret;
	const char *relfroml, *reltol;
	
	relfroml = relfrom;
	reltol   = relto;
	if(relfroml) {
		if(relfroml[0] == '/')
			relfrom++;
	}
	if(reltol) {
		if(reltol[0] == '/')
			reltol++;
	}
	for(int i=0; i< (int) vdirs.size(); i++) {
		ret = vdirs[i]->vdir_replace(relfroml, reltol, from, to, do_fsmv);
		if(ret)
			return ret;
	}
	
	return 0;
}

int  HybfsData::virtual_replace_path(const char *from, const char * to, int brid)
{
	int ret;
	
	if(brid <0 || brid >= (int) vdirs.size())
		return -EINVAL;
	
	ret = vdirs[brid]->vdir_replace_path(from, to);
	
	return ret;
}

int HybfsData::start_db_storage()
{
	int ret;
	
	for(int i=0; i< (int) vdirs.size(); i++) {
		ret = vdirs[i]->init();
		if(ret)
			return ret;
	}
	
	return 0;
}

HybfsData::~HybfsData()
{
	branches.clear();
	try{
	for(int i=0; i< (int) vdirs.size(); i++) {
		delete vdirs[i];
	}
	} catch(exception) {
		PRINT_ERROR("Failed to destroy FS data!\n");
	}
	vdirs.clear();
}

} // namespace hybfs
