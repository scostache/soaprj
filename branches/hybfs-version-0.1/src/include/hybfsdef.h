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

/* branch separator */
#define ROOT_SEP ":"

/* temporary */
#define PATHLEN_MAX 1024

/* mount options keys */
#define KEY_HELP 0

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

/*
 * branch structure
 */
typedef struct
{
	int fd; // to prevent accidental umounts
	char * path;
} hybfs_branch_t;

/*
 * main filesystem structure
 */
typedef struct
{
	int nbranches;
	hybfs_branch_t *branches;
	char *mountp;
	int doexit;
	int retval;
} hybfs_t;

extern hybfs_t hybfs_core;

#endif /* HYBFS_DEF_H */
