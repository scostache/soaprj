/*
 dirops.c - Dir operations
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <unistd.h>
#include "hybfs.h"
#include "misc.h"

#include "path_crawler.hpp"
#include "path_data.hpp"


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
        if(pdata == NULL || pdata->check_path_data() == 0) {
        	res = -EINVAL;
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
        if(pdata == NULL || pdata->check_path_data()) {
        	res = -EINVAL;
        	goto out;
        }
        DBG_PRINT("i make dir %s\n", pdata->abspath_str());
        res = rmdir(pdata->abspath_str());
        
out: 
	if(pc)
		delete pc;
	if(pdata)
		delete pdata;
	
        return res;
}
