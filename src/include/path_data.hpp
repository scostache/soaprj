/* 
path_data.hpp - Wrapper for the path components, including the branch id coresponding for
the current path
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */



#ifndef PATH_DATA_HPP_
#define PATH_DATA_HPP_

#include <string>
#include "misc.h"
#include "hybfs_data.hpp"
#include "path_crawler.hpp"

/**
 * This class contains the relative real path extracted from a query and
 * the absolute real path
 */
class PathData {
private:
	int brid;
	std::string *relpath;
	std::string *abspath;
		
public:
	
	PathData(const char *path, HybfsData *hybfs_core, PathCrawler *pc)
	{
		if(pc == NULL || path == NULL || hybfs_core == NULL) {
			PRINT_ERROR("%s:%d : Null argument!\n", __func__, __LINE__);
			return;
		}
		if(path[0] == '\0') {
			PRINT_ERROR("%s:%d : Invalid path!\n", __func__, __LINE__);
			return;
		}
		
		relpath = extract_real_path(path, pc);
		if(relpath == NULL) {
			return;
		}
			
		abspath = resolve_path(hybfs_core, path, &brid);
	}
	
	~PathData()
	{
		if(relpath != NULL)
			delete relpath;
		if(abspath != NULL)
			delete abspath;
	}
	
	/**
	 * Checks if this class contains valid path data. This checks only if the
	 *  paths are not null.
	 * @param p the structure that contains data about a path
	 */
	int check_path_data()
	{
		return ( (relpath == NULL || abspath == NULL) ? 0 : 1);
	}
	
	const char * abspath_str() { return abspath->c_str(); }
	
	const char * relpath_str() { return relpath->c_str(); }
	
	int 	     get_brid()    { return brid; }

};

#endif /*PATH_DATA_HPP_*/
