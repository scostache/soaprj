/* 
 hybfs_data.h - Class that holds all the main information needed by our FS
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef HYBFS_DATA_H_
#define HYBFS_DATA_H_

#include <vector>
#include <string>

#include "hybfsdef.h"
#include "virtualdir.h"

using namespace std;

class HybfsData
{
private:
	/* the mount directory - not used (yet) */
	char *mountp;
	/* branch paths */
	vector <string> branches;
	/* database handle for each branch */
	vector<VirtualDirectory *> vdirs;

public:
	HybfsData(char *mountp);
	~HybfsData();

	/*
	 * Adds a branch to the overlaid list
	 */
	int add_branch(const char * path);
	
	/*
	 * Deletes a branch from the list
	 */
	int delete_branch(const char * path);
	
	/*
	 * Parses the mount options and adds the branches
	 */
	int parse_branches(const char *arg);
	
	/*
	 * Returns the branch path
	 */
	const char * get_branch_path(int brid);
	
	/* 
	 * Starts the databases
	 */
	int start_db_storage();

	/* does fuse need to exit? */
	int doexit;
	/* the return value for the exit */
	int retval;
};

#endif /*HYBFS_DATA_H_*/
