/*
 fileops.c - File operations
 
 Copyright (C) 2008-2009  Stefania Costache, Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>

#include "hybfs.h"
#include "misc.h"
#include "hybfsdef.h"
#include "path_crawler.hpp"
#include "path_data.hpp"

/* 
 * Warning: the rename is done properly for a SINGLE branch. 
 * How should it change if we have many directories from different
 * file systems underneath us?
 */

static int normal_rename(HybfsData *data, const char *from, const char *to, int rootlen)
{
	string *pf, *pt;
	int ret = 0;
	int brid_to, brid_from;

	DBG_SHOWFC();
	
	/* get absolute paths for both files */
	pf = resolve_path(data, from+rootlen, &brid_from);
	if (pf == NULL)
		return -ENOMEM;
	pt = resolve_path(data, to+rootlen, &brid_to);
	if (pt == NULL)
		return -ENOMEM;
	
	if(brid_to != brid_from) {
		PRINT_ERROR("Cannot do rename between multiple directories!\n");
		return -EINVAL;
	}
	/* do the real rename */
	ret = rename(pf->c_str(), pt->c_str());
	if (ret == 0) {
		ret = data->virtual_replace_query(from, to, brid_from);
	}

	delete pf;
	delete pt;

	return ret;
}

int hybfs_rename(const char *from, const char *to)
{
	int res = 0;
	int nqueries;
	int rootlen;
	string to_copy;
	PathCrawler *pcf = NULL; 
	PathCrawler *pct = NULL;
	
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();
	DBG_PRINT("from: %s to: %s\n", from, to);
	/* sanity checks */
	if(IS_ROOT(from) || IS_ROOT(to))
		return -EINVAL;
	/* For now accept only adding or changing tags for a real path.
	 * A real path should only come from the virtual directory 'path:/' */
	pcf = new PathCrawler(from);
	if(pcf == NULL)
		return -ENOMEM;
	nqueries = pcf->break_queries();
	if(nqueries != 0) {
		res = -EINVAL;
		goto out;
	}
	rootlen = strlen(REAL_DIR);
	if(strncmp(from+1, REAL_DIR,rootlen - 1) !=0) {
		res = -EINVAL;
		goto out;
	}
	/* now check the second path */
	pct = new PathCrawler(to);
	if(pct == NULL) {
		res = -ENOMEM;
		goto out;
	}
	/* If the second is a real path, then do the real rename */
	nqueries = pct->break_queries();
	if(nqueries == 0 && strncmp(to+1, REAL_DIR, rootlen-1) ==0) {
		DBG_PRINT("from %s to %s\n", from, to);
		res = normal_rename(hybfs_core, from, to, rootlen);
		goto out;
	}
	
	to_copy = pct->get_first_path();
	while(pct->has_next_query()) {
		string to_query = pct->pop_next_query();
		
		res = hybfs_core->virtual_addtag(to_query.c_str(), from+rootlen);
		if (res)
			goto out;	
	}
	/* now, if I need to do a real rename, remember to do it here */
	if(to_copy.length() > 0) {
		if(pct->get_first_path().find(REAL_DIR) != 1){
			res = -EINVAL;
			goto out;
		}
		res = normal_rename(hybfs_core, from, to_copy.c_str(), rootlen);
	}
out:
	if(pcf)
		delete pcf;
	if(pct)
		delete pct;
	
	return res;
}


int hybfs_open(const char *path, struct fuse_file_info *fi)
{
        int res, nqueries, fid;
        int rootlen; 
        PathCrawler *pc = NULL;
        PathData *pd;
       
        HybfsData *hybfs_core = get_data();
        
        DBG_SHOWFC();
        
        pc = new PathCrawler(path);
        if(pc == NULL) {
        	res = -ENOMEM;
        	goto out;
        }
        /* check to see if it's at least one query, or if is a real path */
        nqueries = pc->break_queries();
        rootlen = strlen(REAL_DIR);
        fid = -1;
      
        pd = new PathData(path, hybfs_core, pc);
        if(pd == NULL || pd->check_path_data() == 0) {
                res = -ENOMEM;
                goto out;
        }
        
        fid = open(pd->abspath_str(), fi->flags);
        if (fid == -1) {
        	res = -errno;
        	if(res == -ENOENT) {
        		/* TODO: try to delete the info from the db, 
        		 * because it means it was changed underneath us! */
        	}
        	goto out;
        }
        
        /* add the tags to the db for this file if the create flag was specified*/
        if(fi->flags & O_CREAT) {
	        if(nqueries >1) {
	        	res = -EINVAL;
	        	goto out;
	        }
	        res = hybfs_core->virtual_addtag(pc->get_next().c_str(),
	        		pd->relpath_str());
	        if(res)
	        	goto out;
        }
        /* keep this for future use with read/write/close */
        fi->fh = (unsigned long) fid;
        res = 0;
        
out: 
	if(fid >0 && res !=0)
		close(fid);
	if(pc)
		delete pc;
	if(pd)
		delete pd;
	
        return res;
}


int hybfs_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi)
{
        int res;

        DBG_SHOWFC();

        res = pread(fi->fh, buf, size, offset);
        if (res == -1)
        	return -errno;

        return res;
}

int hybfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi)
{
        int res;

        DBG_SHOWFC();

        res = pwrite(fi->fh, buf, size, offset);
        if (res == -1)
        	return -errno;

        return res;
}

int hybfs_truncate(const char *path, off_t size) 
{
	int res, nqueries;
	PathCrawler *pc= NULL;
	PathData *pd;
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}

	nqueries = pc->break_queries();
	
	pd = new PathData(path, hybfs_core, pc);
	if (pd == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	res = truncate(pd->abspath_str(), size);
out:
	if (pc)
		delete pc;
	if (pd)
		delete pd;
	
	return res;
}

int hybfs_flush(const char *path, struct fuse_file_info *fi) 
{
	/* following the model from unionfs-fuse */
	
	int fd = dup(fi->fh);

	if (fd == -1) {
		if (fsync(fi->fh) == -1) return -EIO;
		return -errno;
	}

	int res = close(fd);
	if (res == -1) return -errno;

	return 0;
}

int hybfs_release(const char *path, struct fuse_file_info *fi)
{
        int res;
        
        DBG_SHOWFC();
        
        res = close(fi->fh);
        if (res == -1) 
        	return -errno;

        return 0;
}

int hybfs_utimens(const char *path, const struct timespec ts[2]) 
{
	int res, nqueries;
        struct timeval tv[2];
        PathData *pd = NULL;
        PathCrawler *pc = NULL;
       
        HybfsData *hybfs_core = get_data();
        
        DBG_SHOWFC();
        
        pc = new PathCrawler(path);
        if(pc == NULL) {
        	res = -ENOMEM;
        	goto out;
        }

        nqueries = pc->break_queries();
      
        pd = new PathData(path, hybfs_core, pc);
        if(pd == NULL || pd->check_path_data() == 0) {
        	res = -ENOMEM;
        	goto out;
        }
        
        tv[0].tv_sec = ts[0].tv_sec;
        tv[0].tv_usec = ts[0].tv_nsec / 1000;
        tv[1].tv_sec = ts[1].tv_sec;
        tv[1].tv_usec = ts[1].tv_nsec / 1000;
        	
        res = utimes(pd->abspath_str(), tv);
        
out: 
	if(pc)
		delete pc;
	if(pd)
		delete pd;
	
        return res;
}

int hybfs_chmod(const char *path, mode_t mode) 
{
	int res,nqueries;
	PathData *pd = NULL;
	PathCrawler *pc= NULL;
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}

	nqueries = pc->break_queries();
	pd = new PathData(path, hybfs_core, pc);
	if (pd == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	res = chmod(pd->abspath_str(), mode);
out:
	if (pc)
		delete pc;
	if (pd)
		delete pd;

	return res;
}

int hybfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int res, nqueries;
	PathData *pd;
	PathCrawler *pc= NULL;
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}

	nqueries = pc->break_queries();
	pd = new PathData(path, hybfs_core, pc);
	if (pd == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	res = chown(pd->abspath_str(), uid, gid);
out:
	if (pc)
		delete pc;
	if (pd)
		delete pd;

	return res;	
}

/**
 * Set file owner of after an operation, which created a file.
 */
static int set_owner(const char *path)
{
	struct fuse_context *ctx = fuse_get_context();

	if (ctx->uid != 0 && ctx->gid != 0) {
		int res = lchown(path, ctx->uid, ctx->gid);
		if (res)
			return -errno;
	}

	return 0;
}

int hybfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res, nqueries;
	PathData *pd = NULL;
	PathCrawler *pc= NULL;
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}

	nqueries = pc->break_queries();
	pd = new PathData(path, hybfs_core, pc);
	if (pd == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	res = mknod(pd->abspath_str(), mode, rdev);
	if (res == -1) {
		res = -errno;
		goto out;
	}
	/* add the tags to the db for this file if the create flag was specified*/
	if (nqueries >1) {
		res = -EINVAL;
		goto out;
	}
	res = hybfs_core->virtual_addtag(pc->pop_next_query().c_str(), pd->relpath_str());
	if (res)
		goto out;

	set_owner(pd->abspath_str());

out:
	if (pc)
		delete pc;
	if (pd)
		delete pd;

	return res;
}

int hybfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int res, nqueries, fid;
	PathData *pd = NULL;
	PathCrawler *pc= NULL;
	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}
	/* check to see if it's at least one query, or if is a real path */
	nqueries = pc->break_queries();
	fid = -1;

	pd = new PathData(path, hybfs_core, pc);
	if (pc == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	
	fid = open(pd->abspath_str(), fi->flags, mode);
	if (fid == -1) {
		res = -errno;
		goto out;
	}

	/* add the tags to the db for this file if the create flag was specified*/
	if (nqueries >1) {
		res = -EINVAL;
		DBG_SHOWFC();
		goto out;
	}
	res = hybfs_core->virtual_addtag(pc->pop_next_query().c_str(), pd->relpath_str());
	if (res)
		goto out;

	/* keep this for future use with read/write/close */
	fi->fh = (unsigned long) fid;
	res = 0;

	/* no error check, since creating the file succeeded */
	set_owner(pd->abspath_str());

out: 
	if (fid >0 && res !=0)
		close(fid);
	if (pc)
		delete pc;
	if (pd)
		delete pd;
	
	return res;
}

int hybfs_unlink(const char *path)
{
	int res, nqueries;
	PathData *pd = NULL;
	PathCrawler *pc= NULL;
	HybfsData *hybfs_core = get_data();
	
	DBG_SHOWFC();

	pc = new PathCrawler(path);
	if (pc == NULL) {
		res = -ENOMEM;
		goto out;
	}
	/* check to see if it's at least one query, or if is a real path */
	nqueries = pc->break_queries();
	
	pd = new PathData(path, hybfs_core, pc);
	if (pd == NULL || pd->check_path_data() == 0) {
		res = -ENOMEM;
		goto out;
	}
	DBG_PRINT("unlink path is %s\n", pd->abspath_str());
	res = unlink(pd->abspath_str());
	if (res) {
		res = -errno;
		if(res == -ENOENT) {
			
			res = hybfs_core->virtual_remove_file(pd->relpath_str(),
					pd->get_brid());
		}
		goto out;
	}
	
	res = hybfs_core->virtual_remove_file(pd->relpath_str(), pd->get_brid());
	if (res)
		goto out;
	
out: 
	if (pc)
		delete pc;
	if (pd)
		delete pd;

	return res;
}
