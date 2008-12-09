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
enum tag_op {
	TAG_ADD,
	TAG_REPLACE,
	TAG_REMOVE
};


/* path tokenizer, useful when separating conjunctions */
typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;


typedef int (*filler_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);

/**
 * wrapper for struct stat - this is needed by the fuse filler
 */
typedef struct stat stat_t;

extern int fs_rename(const char *from, const char * to);

/**
 * structure that holds the information stored in the db about a file.
 * With Sqlite is not so useful, but it may be needed more when we will switch
 * to BDB.
 * (deprecated) - it should be changed to new_file_info_t, but I keep it for 
 * compatibility with the extenstions framework
 */
typedef struct
{
	int brid;
	__ino64_t fid;
	__mode_t mode;
	int namelen;
	char name[0];
} file_info_t;

/**
 * structure that holds info about a tag. I use strings for now, but it's not good
 */
typedef struct tag_info_t
{
	std::string tag;
	std::string value;
} tag_info_t;

/* for debugging */

#ifdef DBG

#define DBG_PRINT(...) 					  \
	do {						  \
		printf("%s(): %d: ", __func__, __LINE__); \
		printf(__VA_ARGS__);			  \
	} while (0);
#define DBG_SHOWFC() printf("%s(): %d: \n", __func__, __LINE__);

#else

#define DBG_PRINT(...) 
#define DBG_SHOWFC()

#endif

#define PRINT_ERROR(...) fprintf(stderr, __VA_ARGS__);

#define ABORT(cond,message) \
	if(cond) { fprintf(stderr, "Abort from function %s : %s\n",__func__,message); \
	exit(1); }

#define IS_ROOT(path) \
	((strcmp(path, REAL_DIR) == 0 || (strncmp(path, REAL_DIR, strlen(REAL_DIR) -1) == 0 \
		&& strlen(path) == strlen(REAL_DIR) -1)) ? 1 : 0)

#endif /* HYBFS_DEF_H */

