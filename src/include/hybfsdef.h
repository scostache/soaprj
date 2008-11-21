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

/* branch separator */
#define ROOT_SEP ':'

/* temporary */
#define PATHLEN_MAX 1024

/* mount options keys */
#define KEY_HELP 0

/* virtual directory for showing what is underneath us */
#define REAL_DIR "path:/"

typedef boost::tokenizer<boost::char_separator<char> > path_tokenizer;

typedef struct
{
	int brid;
	__ino64_t fid;
	__mode_t mode;
	int namelen;
	char name[0];
} file_info_t;

typedef struct
{
	int brid;
	int pathlen;	
	int namelen;	
	char abspath[0]; /* the path and name packed together */
} path_info_t;

typedef struct
{
	int tagelen;
	char tag[0];
} tag_info_t;


#endif /* HYBFS_DEF_H */

