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
#include "hybfsdef.h"
#include "misc.h"
#include "path_crawler.hpp"

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
	
	return ret;
}

int HybfsData::virtual_addtag(const char* tag, const char* path)
{
	string *abspath;
	int brid;
	int res, taglen, pathlen;
	string stag;
	file_info_t *finfo = NULL;
	struct stat st;
	
	vector <string> *tags = new vector<string>;
	boost::char_separator<char> sep("+ ");
	
	
	taglen = strlen(tag);
	pathlen = strlen(path);
	memset(&st, 0, sizeof(struct stat));
	
	/* the tag, or conjunction of tags must come surrounded by '(' and ')' */
	if(tag[0] != '(' || tag[taglen-1] != ')') {
		delete tags;
		return -EINVAL;
	}
	/* strip the tag */
	stag.assign(tag+1, taglen-2);
	/* break the query in multiple tags */
	path_tokenizer tokens(stag, sep);
		
	/* get the absolute path */
	abspath = resolve_path(this, path, &brid);
	if(abspath == NULL) {
		res = -ENOMEM;
		goto out;
	}
	DBG_PRINT("I have tags: %s for path %s\n", tag, abspath->c_str());
	res = lstat(abspath->c_str(), &st);
	if (res) {
		res = -errno;
		goto out;
	}
	
	finfo = (file_info_t *) malloc(sizeof(file_info_t) + pathlen);
	if(finfo == NULL) {
		res = -ENOMEM;
		goto out;
	}
	/* the path always comes absolute */
	finfo->namelen = pathlen;
	memcpy(&finfo->name[0], path+1, pathlen);
	finfo->name[pathlen-1] = '\0';
	
	finfo->fid = st.st_ino;
	finfo->mode = st.st_mode;
	
	 for (path_tokenizer::iterator tok_iter = tokens.begin();
	       tok_iter != tokens.end(); ++tok_iter) {
		 DBG_PRINT(" I have: #%s# \n", (*tok_iter).c_str());
		 tags->push_back(*tok_iter);
	 }
	res = vdirs[brid]->vdir_add_tag(tags, finfo);

out:	
	if(abspath)
		delete abspath;
	if(tags) {
		tags->clear();
		delete tags;
	}
	if(finfo)
		free(finfo);
	
	return res;
}

int HybfsData::virtual_replace_query(const char *oldq, const char *newq, int brid)
{
	return vdirs[brid]->vdir_replace(oldq, newq);
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
