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
#include <sys/time.h>
#include <time.h>


#include "hybfs.h"
#include "hybfsdef.h"
#include "misc.h"


std::string* extract_real_path(const char *path, PathCrawler *pc)
{
	int rootlen, nqueries;
	string *relpath;
	
	if(pc == NULL || path == NULL)
		return NULL;
	if(path[0] == '\0')
		return NULL;
	
	relpath = new string();
	
	rootlen = strlen(REAL_DIR);
	nqueries = pc->get_nqueries();
	
	DBG_PRINT("PATH IS #%s#\n", path);
	/* I have only a real path, or a combination of real path + query */
	if((int) strlen(path+1) >= rootlen-1) {
		if (nqueries == 0 && strncmp(path+1, REAL_DIR, rootlen-1) == 0)
			relpath->assign(path+rootlen);
		else if (pc->get_first_path().length() > 0 && pc->is_real())
			relpath->assign(pc->get_first_path().substr(rootlen, string::npos));
	}
	
	/* do I have a remain at the end? If yes, append it */
	if (pc->get_rel_path().length()>0)
		relpath->append(pc->get_rel_path());
	/* now get the real path - if I have nothing, return nothing */
	if (relpath->length() <1) {
		delete relpath;
		return NULL;
	}

	return relpath;
}


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
	if(abspath == NULL)
		return NULL;
	
	abspath->append(path);

	return abspath;
}


void break_tag(std::string *tag_value, std::string *tag, std::string *value)
{
	size_t fpos = tag_value->find(":");
	
	if(fpos != string::npos) {
		tag->assign(tag_value->substr(0, fpos));
		value->assign(tag_value->substr(fpos+1, tag_value->length() - 1));
	}
	else
		tag->assign(*tag_value);
}

int parse_tags(std::string *query, vector<std::string> *tags, int *op_type)
{
	int res;
	boost::char_separator<char> sep(" +()", "|!");
	path_tokenizer t(*query, sep);
	
	DBG_PRINT("I have path %s\n", query->c_str());
	
	/* check to see if I have a query surrounded by '(' and ')' */
	if(query->at(0) != '(' || query->at(query->length()-1) != ')')
		return -1;
	
	res = 0;
	/* get the second element, and see if it's ! or | */
	path_tokenizer::iterator beg=t.begin();
	switch((*beg)[0]) {
		case '!':
			*op_type = TAG_REMOVE;
			break;
		case '|':
			*op_type = TAG_ADD;
			break;
		case '+':
			*op_type = TAG_REPLACE;
			break;
		default:
			*op_type = TAG_REPLACE;
			tags->push_back(*beg);
			break;
	}
	DBG_PRINT(" First element of path is #%s# \n",  (*beg).c_str());
	beg++;
	/* now that we set our operation, get the tags */
	for(; beg!=t.end();beg++) {
		if((*beg).length() == 0)
			continue;
		if((*beg)[0] == '!' || (*beg)[0] == '|' ) {
			/* why would anyone specify smth like this in a 
			 * conjunction? */
			res = -1;
			tags->clear();
			break;
		}
		DBG_PRINT("I have tag %s\n", (*beg).c_str());
		tags->push_back(*beg);
	}
	
	return res;
}

/* filler for the stat structure */
int get_stat(const char *path, stat_t *buf)
{
	int ret;
	
	memset(buf, 0, sizeof(*buf));
	ret = lstat(path, (struct stat *)buf);
	if(ret)
		PRINT_ERROR("Cannot stat file %s\n", path);
	return ret;
}

void fill_dummy_stat(stat_t *st)
{
	struct timeval tmv;
	
	gettimeofday(&tmv, NULL);
	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFDIR | 0755;
	st->st_nlink = 2;
	st->st_uid = getuid();
	st->st_gid = getgid();
	
	st->st_atime = st->st_mtime = st->st_ctime = tmv.tv_sec;
}
