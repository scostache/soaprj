/* 
 hybfs_data.hpp - Class that holds all the main information needed by our FS
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef HYBFS_DATA_HPP_
#define HYBFS_DATA_HPP_

#include <vector>
#include <string>

#include "hybfsdef.h"
#include "virtualdir.hpp"

using namespace std;

/**
 * Class that holds all the main information needed by our FS
 */
class HybfsData
{
private:
	/**
	 *  the mount directory - not used (yet) 
	 */
	char *mountp;
	/**
	 *  branch paths 
	 */
	vector <string> branches;
	/**
	 *  database handle for each branch 
	 */
	vector<VirtualDirectory *> vdirs;

public:
	HybfsData(char *mountp);
	~HybfsData();

	/**
	 * Adds a branch to the overlaid list
	 */
	int add_branch(const char * path);
	
	/**
	 * Deletes a branch from the list
	 */
	int delete_branch(const char * path);
	
	/**
	 * Parses the mount options and adds the branches
	 */
	int parse_branches(const char *arg);
	
	/**
	 * Returns the branch path
	 */
	const char * get_branch_path(int brid);
	
	/**
	 * Returns the number of branches
	 */
	int get_nbranches() { return branches.size(); }
	
	/** 
	 * Starts the databases
	 */
	int start_db_storage();
	
	/**
	 * Get the number of links from under us
	 */
	int get_nlinks();
	
	/**
	 * virtual readdir for each branch that we have
	 */
	int virtual_readdir(const char * query, void *buf, filler_t filler);
	
	/**
	 * Adds a tag for this path to the corresponding db. The path is relative and it will
	 * be changed to absolute here.
	 */
	int virtual_addtag(const char* tag, const char* path);
	
	/**
	 * This is a sort of rename/move but for tags. It can also change the path of a file
	 * from a database, since it is seen as a special value of the tag 'path:'
	 */
	int virtual_replace_query(const char *oldq, const char *newq, int brid);

	/**
	 *  does fuse need to exit? Set this to 1 if yes.
	 */
	int doexit;
	
	/**
	 *  the return value at exit 
	 */
	int retval;
};

#endif /*HYBFS_DATA_HPP_*/
