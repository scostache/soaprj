/* 
 db_ops.c - Berkley DB access methods
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <stdlib.h>

#include "db_ops.h"
#include "hybfs.h"
#include "misc.h"

hybfs_db_t hybfs_db;

static int init_env()
{
	char * env_home_dir;
	int ret;
	u_int32_t env_flags;

	ret = db_env_create(&hybfs_db.core_env, 0);
	if (ret != 0) {
		fprintf(stderr, "Error creating env handle: %s\n",
		                db_strerror(ret));
		return -1;
	}

	env_flags = DB_THREAD | DB_CREATE | DB_INIT_MPOOL;
	
	/* FIXME here we set our enviroment only in the first branch */
	env_home_dir = concat_paths(hybfs_core.branches[0].path, METADIR);
	ret = hybfs_db.core_env->open(hybfs_db.core_env, env_home_dir ,env_flags, 0);
	free(env_home_dir);
	
	if (ret != 0) {
		fprintf(stderr, "Environment open failed: %s",
		db_strerror(ret));
		return -1;
	}

	return 0;
}

int init_db_storage()
{
	int ret = 0;
	u_int32_t db_flags;
	
	/* init the enviroment(s) */
	ret = init_env();
	if( ret != 0)
		return ret;
	
	ret = db_create(&hybfs_db.main_table, hybfs_db.core_env, 0);
	if(ret != 0) {
		fprintf(stderr, "Error creating db handle: %s",
				db_strerror(ret));
		goto on_error;
	}
	
	/* init the databases */
	db_flags = DB_THREAD | DB_CREATE;

	ret = hybfs_db.main_table->open(hybfs_db.main_table,
	                NULL, MAINDB, NULL, DB_HASH, db_flags, 0);
	if (ret != 0) {
		fprintf(stderr, "Error opening db handle: %s",
				db_strerror(ret));
	  goto on_error;
	}
	
	return 0;
	
on_error:
	close_db_storage();
	ret = -1;
	
	return ret;
}

void close_db_storage()
{
	/* close all databases here */
	if(hybfs_db.main_table != NULL)
		hybfs_db.main_table->close(hybfs_db.main_table,0);
	
	/* close the enviroment(s) */
	if(hybfs_db.core_env != NULL)
		hybfs_db.core_env->close(hybfs_db.core_env, 0);
}
