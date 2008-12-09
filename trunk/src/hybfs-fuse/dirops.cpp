/*
 dirops.c - Dir operations
 
 Copyright (C) 2008-2009  Stefania Costache
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <unistd.h>
#include "hybfs.h"

#include "core/misc.h"
#include "core/path_crawler.hpp"
#include "core/path_data.hpp"


int hybfs_mkdir(const char *path, mode_t mode)
{
        int res, nqueries;
        PathCrawler *pc = NULL;
        PathData *pdata = NULL;
       
        HybfsData *hybfs_core = get_data();
        
        DBG_SHOWFC();
        
        pc = new PathCrawler(path);
        if(pc == NULL) {
        	res = -ENOMEM;
        	goto out;
        }

        nqueries = pc->break_queries();
       
        pdata = new PathData(path, hybfs_core, pc);
        if(pdata == NULL) {
        	res = -EPERM;
        	goto out;
        }
        if(pdata ->check_path_data() == 0) {
        	res = 0;
        	goto out;
        }
        DBG_PRINT("i make dir %s\n", pdata->abspath_str());
        res = mkdir(pdata->abspath_str(), mode);
        
out: 
	if(pc)
		delete pc;
	if(pdata)
		delete pdata;
	
        return res;
}

int hybfs_rmdir(const char *path)
{
        int res, nqueries;
        PathCrawler *pc = NULL;
        PathData *pdata = NULL;
       
        HybfsData *hybfs_core = get_data();
        
        DBG_SHOWFC();
        
        pc = new PathCrawler(path);
        if(pc == NULL) {
        	res = -ENOMEM;
        	goto out;
        }

        nqueries = pc->break_queries();
       
        pdata = new PathData(path, hybfs_core, pc);
        if(pdata == NULL) {
        	res = -ENOMEM;
        	goto out;
        }
        if(pdata->check_path_data() == 0) {
        	/* Is this a query? Because if it's so, then there is no
        	 * point in calling rmdir */
        	res = 0;
        	goto out;
        }
        
        DBG_PRINT("i remove dir %s\n", pdata->abspath_str());
        res = rmdir(pdata->abspath_str());
        
out: 
	if(pc)
		delete pc;
	if(pdata)
		delete pdata;
	
        return res;
}
