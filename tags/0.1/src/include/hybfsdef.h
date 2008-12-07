/* 
 hybfsdef.h - HybFs structures and definitions

 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef HYBFS_DEF_H
#define HYBFS_DEF_H

#include <sys/types.h>
#include <fuse.h>
#include <boost/tokenizer.hpp>

/**
 *  branch separator 
 */
#define ROOT_SEP ':'

/* temporary */
#define PATHLEN_MAX 1024

/**
 *  mount options keys 
 */
#define KEY_HELP 0

/**
 *  virtual directory for showing what is underneath us 
 */
#define REAL_DIR "path:/"

/**
 *  special value for a tag that ... has no value :D 
 */
#define NULL_VALUE "null"

/**
 * Define the types of operations on tags - this is useful when we want to
 * update the tags from the DB and we decide to remove, add or replace them
 */
#define TAG_ADD     0
#define TAG_REPLACE 1
#define TAG_REMOVE  2

/* path tokenizer, useful when separating conjunctions */
typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;


/**
 * function called when wanting to fill a dir buffer. We declare it because
 * it may be subject of change (?) 
 */
typedef fuse_fill_dir_t filler_t;

/**
 * wrapper for struct stat - this is needed by the fuse filler
 */
typedef struct stat stat_t;

/**
 * structure that holds the information stored in the db about a file.
 * With Sqlite is not so useful, but it may be needed more when we will switch
 * to BDB.
 */
typedef struct
{
	int brid;
	__ino64_t fid;
	__mode_t mode;
	int namelen;
	char name[0];
} file_info_t;


#endif /* HYBFS_DEF_H */

