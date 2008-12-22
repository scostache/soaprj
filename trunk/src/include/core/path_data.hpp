/* 
path_data.hpp - Wrapper for the path components, including the branch id 
coresponding for the current path
 
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

namespace hybfs {

/**
 * @class PathData
 * @brief
 * Class that contains the relative and the absolute real paths extracted from a
 *  query string. It also builds the real path from the underlying file system.
 */
class PathData {
private:
	/**
	 * @brief The branch identifier
	 */
	int brid;
	/**
	 * @brief
	 * The relative path extracted with the help of a PathCrawler class.
	 */
	std::string *relpath;
	/**
	 * @brief
	 * The real path from the underlying file system
	 */
	std::string *abspath;
		
public:
	
	PathData(const char *path, HybfsData *hybfs_core, PathCrawler *pc)
	{
		relpath = NULL;
		abspath = NULL;
		
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
			
		abspath = resolve_path(hybfs_core, relpath->c_str(), &brid);
	}
	
	~PathData()
	{
		if(relpath != NULL)
			delete relpath;
		if(abspath != NULL)
			delete abspath;
	}
	
	/**
	 * @brief
	 * Checks if this class contains valid path data. This checks only if the
	 *  paths are not null.
	 * @return Returns 1 if the path is valid (it may represent a real path)
	 * and 0 otherwise.
	 */
	int check_path_data()
	{
		return ( (relpath == NULL || abspath == NULL) ? 0 : 1);
	}
	
	/**
	 * @brief Returns the absolute path as a char pointer. It must not be freed.
	 */
	const char * abspath_str() { return (abspath) ? abspath->c_str() : NULL; }
	
	/**
	 * @brief Returns the relative path as a char pointer. It must not be freed.
	 */
	const char * relpath_str() { return (relpath) ? relpath->c_str() : NULL; }
	
	/**
	 * @brief Returns the branch identifier.
	 */
	int 	     get_brid()    { return brid; }

};

}

#endif /*PATH_DATA_HPP_*/
