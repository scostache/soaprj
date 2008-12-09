/*
 fileops.c - File operations
 
 Copyright (C) 2008-2009  Stefania Costache

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
#include "core/misc.h"
#include "core/hybfsdef.h"
#include "core/path_crawler.hpp"
#include "core/path_data.hpp"

/* 
 * Warning: the rename is done properly for a SINGLE branch. 
 * How should it change if we have many directories from different
 * file systems underneath us?
 */


int fs_rename(const char *from, const char *to)
{
	int res;
	
	res = rename(from, to);
	if(res)
		res = -errno;
	
	return res;
}

/**
 * This does rename on the underlying fs. Also it changes the file paths from the
 * db. But these are the only things that it does!
 */
static int normal_rename(HybfsData *data, PathData *from, PathData *to)
{
	const char *pf, *pt;
	int ret = 0;
	int brid_to, brid_from;

	DBG_SHOWFC();
	
	pf = from->abspath_str();
	pt = to->abspath_str();
	
	brid_from = from->get_brid();
	brid_to   = to->get_brid();
	
	if(brid_to != brid_from) {
		PRINT_ERROR("Cannot do rename between multiple directories!\n");
		return -EINVAL;
	}
	/* do the real rename */
	if(pf != NULL && pt != NULL) {
		/* path to path */
		ret = rename(pf, pt);
		if (ret == 0) {
			ret = data->virtual_replace_path(from->relpath_str(), 
					to->relpath_str(), brid_from);
		}
	}
	
	return ret;
}

/**
 * The syntax for this operation is as following:
 * @param from  this argument can be a query or a real path. A query can contain
 *  a real path, of course, but then it becomes a conjunction from the query 
 * ( we take recursively all the files from the real path that match the desired
 *  query).
 * 
 * @param to this parameter explains what will actually happen to the file(s). It
 * will be an operation based only on tags if there is no real path component 
 * specified. If this exists, then a rename will also happen.
 * 
 * If the from path is 100% real, then all the ops will be performed on this. 
 * We are NOT supporting tags ops for directories (not yet).
 * If both the from and to paths are real (no queries) then we rely on the underlying
 * rename operation. Otherwise, we have to do it ourselves, and search in the database
 * for the file paths that do a match on the query.
 */
int hybfs_rename(const char *from, const char *to)
{
	int res = 0;
	int nqf, nqt, isdir, isreal;
	string to_copy;
	struct stat st;
	PathData *pdf = NULL;
	PathData *pdt = NULL;
	PathCrawler *pcf = NULL; 
	PathCrawler *pct = NULL;
	HybfsData *hybfs_core = get_data();

	DBG_SHOWFC();
	DBG_PRINT("from: %s to: %s\n", from, to);
	/* sanity checks */
	if(IS_ROOT(from) || IS_ROOT(to))
		return -EINVAL;

	pcf = new PathCrawler(from);
	if(pcf == NULL)
		return -ENOMEM;
	nqf = pcf->break_queries();
	
	/* now check the second path */
	pct = new PathCrawler(to);
	if(pct == NULL) {
		res = -ENOMEM;
		goto out;
	}
	nqt = pct->break_queries();
	isreal = (pct->is_real() || pct->get_nqueries() == 0);
	
	pdf = new PathData(from, hybfs_core, pcf);
	pdt = new PathData(to,   hybfs_core, pct);
	if(pdf == NULL || pct == NULL) {
		res = -ENOMEM;
		goto out;
	}
	
	/* test if is a directory or a file.*/
	isdir = 0;
	if(pdf->abspath_str() ==  NULL) {
		isdir = 1; /* think of a query as a virtual dir */
	} else {
		memset(&st,0, sizeof(struct stat));
		res = stat(pdf->abspath_str(), &st);
		if(res) {
			res = -errno;
			goto out;
		}
		/*even if it's a directory, treat it like a query-to-query*/
		if(S_ISDIR(st.st_mode)) {
			isdir = 1;
		}
	}
	
	/* done checking, now treat the dir case different: */
	if(isdir) {
		res = hybfs_core->virtual_replace_query(pdf->relpath_str(), 
				pdt->relpath_str(), pcf, pct, isreal);
		goto out;
	}
	
	/* if it's a file, update the tags for it, even if the path is a query (?) */
	if(!isdir) {
		res = hybfs_core->virtual_updatetags(pct, pdf->relpath_str(),
				pdf->abspath_str(), pdf->get_brid());
	}
	
	/* do the real rename for a specified path, if any */
	if(isreal)
		res = normal_rename(hybfs_core, pdf, pdt);
	
out:
	if(pcf)
		delete pcf;
	if(pct)
		delete pct;
	if(pdf)
		delete pdf;
	if(pdt)
		delete pdt;
	
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
	        res = hybfs_core->virtual_addtag(pc, pd->relpath_str(),
	        		pd->abspath_str(), pd->get_brid());
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
	res = hybfs_core->virtual_addtag(pc, pd->relpath_str(),
			pd->abspath_str(), pd->get_brid());
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
	
	DBG_PRINT("absolute path is %s rel path is %s\n",pd->abspath_str(),
			pd->relpath_str());
	fid = open(pd->abspath_str(), fi->flags, mode);
	if (fid == -1) {
		res = -errno;
		goto out;
	}

	/* add the tags to the db for this file if the create flag was specified*/
	res = hybfs_core->virtual_addtag(pc, pd->relpath_str(),
			pd->abspath_str(), pd->get_brid());
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
