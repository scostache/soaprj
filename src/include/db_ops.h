/* 
db_ops.h - Berkley DB access methods and file info structures
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef DB_OPS_H_
#define DB_OPS_H_

#include <db.h>

/* meta dir path*/
#ifndef METADIR
#define METADIR ".hybfs/"
#endif

/*  databases names */
#ifndef MAINDB
#define MAINDB  "main.db"
#endif

typedef struct {
	int fid;
	int brid;
	int namelen;
	char name[0];
} file_info_t;

typedef struct {
	int brid;
	int pathlen;
	int namelen;
	char abspath[0];
} path_info_t;

typedef struct {
	int tagelen;
	char tag[0];
} tag_info_t;


/* main database structure */
typedef struct{
	DB_ENV 	  *core_env;	// the core enviroment
	DB 	  *main_table; // the main database
} hybfs_db_t;

extern hybfs_db_t hybfs_db;

int init_db_storage();

void close_db_storage();

#endif /*DB_OPS_H_*/
