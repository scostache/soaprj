/* 
 virtualdir.cpp - Wrapper for virtual directory operations
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <vector>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <fuse.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>


#include "misc.h"
#include "hybfs.h"
#include "hybfsdef.h"

#include "db_backend.hpp"
#include "virtualdir.hpp"
#include "path_crawler.hpp"

using namespace boost;

typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;


VirtualDirectory::VirtualDirectory(const char *path)
{
	struct stat buf;
	string abspath = path;
	
	vdir_path.assign(path);
	
	abspath.append(METADIR);
	db = NULL;
	
	/* check if the directory exists, if not - create */
	if (lstat(abspath.c_str(), &buf) == -1) {
		DBG_PRINT("Error at checking directory! Atempt to create one!\n");
		/* TODO get appropriate permisions for our directory */
		if (mkdir(abspath.c_str(), 0755)) {
			perror("Failed to create directory: ");
		}
	}
	abspath.append(MAINDB);

	db = new DbBackend(abspath.c_str(), path);
}

VirtualDirectory::~VirtualDirectory()
{
	
	delete db;
}

int VirtualDirectory::check_for_init()  {
	return (db == NULL) ? -1 :0;
}

int VirtualDirectory::vdir_add_tag(PathCrawler *pc, file_info_t *finfo)
{
	int op;
	int res;
	string stag;
	vector<string> *tags = new vector<string>;

	while (pc->has_next_query()) {
		stag = pc->pop_next_query();
		res = parse_tags(&stag, tags, &op);
		if ((op != TAG_REPLACE && op!= TAG_ADD) || res != 0) {
			PRINT_ERROR("Invalid tag operation for a file %s:%d",
					__func__,__LINE__);
			res = -EINVAL;
			goto out;
		}
	}

	res = db->db_add_file_info(tags, finfo, 0);

out:	
	if(tags) {
		tags->clear();
		delete tags;
	}

	return res;
}

int VirtualDirectory::vdir_remove_file(const char *path)
{
	int res;
	const char *lpath;
	
	if(path == NULL)
		return -EINVAL;
	if(path[0] == '\0')
		return -EINVAL;
	
	lpath = path;
	if(path[0] == '/')
		lpath = path + 1;
	
	res = db->db_delete_file_info(lpath);
	
	if(res)
		res = -EINVAL;
	
	return res;
}

int VirtualDirectory::update_file(vector<string> *tags, int op, file_info_t *finfo,
                                  int exist)
{
	int res = 0;
	
	switch(op) {
	case TAG_ADD:
		res = db->db_add_file_info(tags, finfo, exist);
		break;
	case TAG_REPLACE:
		res = db->db_update_file_tags(tags, finfo, exist);
		break;
	case TAG_REMOVE:
		res = db->db_delete_file_tags(tags, finfo);
		break;
	default:
		res = -1;
	}
	
	return res;
}

int VirtualDirectory::vdir_replace(const char*relfrom, const char *relto,
                                   PathCrawler *from, PathCrawler *to, int do_fsmv)
{
	int res = 0;
	int nfiles, ntagops, op;
	string *sql_query;
	string stag;
	string *path = NULL;
	vector<new_file_info_t> files;
	vector<tags_op_t> tagops;
	file_info_t *finfo = NULL;
	
	if(relfrom)
		path = new string(relfrom);
	
	/* Get the file names for this query and update tags, and do a rename (?) */
	sql_query = from->db_build_sql_query(NULL);
	res = db->get_file_names(sql_query, path, &files);
	if(files.size() == 0) {
		res = -ENOENT;
		goto out;
	}
	
	while (to->has_next_query()) {
		vector<string> tags; 
		tags_op_t tagop;
		stag = to->pop_next_query();
		res = parse_tags(&stag, &tags, &op);
		if (res != 0) {
			PRINT_ERROR("Invalid tag operation for a file %s:%d",
					__func__,__LINE__);
			res = -EINVAL;
			goto out;
		}
		tagop.tags = tags;
		tagop.op   = op;
		tagops.push_back(tagop);
	}
	
	nfiles = files.size();
	ntagops = tagops.size();
	for(int i=0; i< nfiles; i++) {
		res = 0;
		finfo = (file_info_t*) malloc(sizeof(*finfo) + files[i].path.length() +1);
		finfo->namelen = files[i].path.length();
		finfo->fid     = files[i].ino;
		memcpy(&finfo->name[0], files[i].path.c_str(), finfo->namelen);
		finfo->name[finfo->namelen] = '\0';	
		
		for(int j = 0; j<ntagops; j++) {
			res = update_file(&tagops[j].tags, tagops[j].op, finfo,1);
			tagops[j].tags.clear();
			if(res)
				break;
		}
		if(res == 0 && relto != NULL && do_fsmv) {
			// build the real path(s) 
			string absfrom = files[i].path;
			string to = files[i].path;
			string absto = files[i].path;
			if(relfrom != NULL)
				replace_first(to, relfrom, relto);
			else {
				size_t pos = to.find_last_of('/');
				if(pos != string::npos)
					to.erase(0,pos);
				if(relto != NULL) {
					if(relto[strlen(relto)-1] != '/')
						to.insert(0,"/");
					to.insert(0,relto);	
				}
			}
			absto.assign(vdir_path);
			absto.append(to);
			absfrom.insert(0,vdir_path);
		
			DBG_PRINT("I move file %s to %s\n", absfrom.c_str(),
			         absto.c_str());
			
			res = fs_rename(absfrom.c_str(), absto.c_str());
			if(res == 0)
				db->update_file_path(files[i].path.c_str(), to.c_str());
		}
		free(finfo);
		if(res)
			break;
		
	}
out:		
	if(res == -1)
		res = -EINVAL;
	if(path)
		delete path;
	tagops.clear();
	files.clear();
	delete sql_query;
	
	return res;
	
}

int VirtualDirectory::vdir_update_tags(PathCrawler *from, file_info_t *finfo)
{
	int op;
	int res = 0;
	string stag;
	vector<string> *tags = new vector<string>;

	while (from->has_next_query()) {
		tags->clear();
		stag = from->pop_next_query();
		res = parse_tags(&stag, tags, &op);
		if (res != 0) {
			PRINT_ERROR("Invalid tag operation for a file %s:%d",
					__func__,__LINE__);
			res = -EINVAL;
			goto out;
		}
		res = update_file(tags, op, finfo, 0);
		if(res) {
			res = -EINVAL;
			goto out;
		}
	}

out:	
	if(tags) {
		tags->clear();
		delete tags;
	}

	return res;
}

int VirtualDirectory::vdir_replace_path(const char *from, const char *to)
{
	int res;
	const char *froml, *tol;
	
	froml = from;
	tol   = to;
	
	if(froml[0] == '/')
		froml++;
	if(tol[0] == '/')
		tol++;
	
	res = db->update_file_path(froml, tol);
	
	if(res)
		return -EINVAL;
	
	return 0;
}

static int fill_buf(list<string> *tags, void *buf, filler_t filler)
{
	for (list<string>::const_iterator i = tags->begin(); i != tags->end(); ++i) {
		if (filler(buf, (*i).c_str(), NULL, 0))
			return 1;
	}

	return 0;
}

int VirtualDirectory::vdir_list_root(const char * path, void *buf, filler_t filler)
{
	list<string> *tags;
	int res = 0;
	
	try {
		/* get all the simple tags */
		tags = db->db_get_tags(path);

		if(tags == NULL)
			throw std::bad_alloc();
		res = fill_buf(tags, buf, filler);
		tags->clear();
		delete tags;
		if(res)
			return 0;
	
		/* and all the tag:value pairs */
		tags = db->db_get_tags_values(path);
		
		if(tags == NULL)
			throw std::bad_alloc();
	
		res = fill_buf(tags, buf, filler);
		tags->clear();
		delete tags;
	}
	catch(std::exception) {
		PRINT_ERROR("Hybfs: Not enough memory! %s line %d\n",
				__func__,__LINE__);
		return -ENOMEM;
	}
	/* res is the result from our filler*/
	
	return 0;
}

int VirtualDirectory::vdir_readdir(const char * query, void *buf,
                                   filler_t filler)
{
	int res = 0;
	PathCrawler *pc;
	string *path_query;
	string *sql_query;
	vector<tag_info_t> *tags = NULL;

	/* is this an empty query? */
	if (query[0] == '\0') {
		PRINT_ERROR("NULL query; use readroot instead! \n");
		return -EINVAL;
	}

	/* get all the queries that are relative to this one in a queue */
	pc = new PathCrawler(query);
	if(pc == NULL) {
		return -ENOMEM;
	}
	/* we have a path like /dir/dir1/.. and it's invalid */
	if(pc->break_queries() == 0) {
		delete pc;
		
		return -ENOENT;
	}
	
	res = 0;
	path_query = extract_real_path(query, pc);
	
	if(path_query != NULL)
	DBG_PRINT("first path is %s\n", path_query->c_str());
	
	tags = new vector<tag_info_t>;
	sql_query = pc->db_build_sql_query(tags);
	res = db->db_get_filesinfo(sql_query, tags, path_query, buf, filler);
	
	delete pc;
	tags->clear();
	delete tags;
	
	if(path_query)
		delete path_query;
	if(sql_query)
		delete sql_query;
	
	return (res == -1) ? -EIO : 0;
}
