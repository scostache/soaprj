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
#include "hybfsdef.h"

/* meta dir path*/
#ifndef METADIR
#define METADIR ".hybfs"
#endif

/*  databases names */
#ifndef MAINDB
#define MAINDB  "main.db"
#endif

/* main database structure */
typedef struct
{
	/* the core enviroment. It is used for caching, and it keeps all our tables */
	DB_ENV *core_env;

	/* the main db: this stores information about the names of the files
	 * associated with a tag. Actually, it's a Hash Table. */
	DB *main_table;
} hybfs_db_t;

extern hybfs_db_t hybfs_db;

int db_init_storage();

void db_close_storage();

int db_add_file_info(char *tag, file_info_t * finfo);

int db_get_file_info(char *tag, DBC *pcursor, int first, file_info_t **finfo);

int db_delete_allfile_info(char *tag);

int db_check_tag(char *tag);

#endif /*DB_OPS_H_*/
