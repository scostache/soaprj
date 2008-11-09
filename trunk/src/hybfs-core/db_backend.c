/* 
 db_ops.c - Berkley DB access methods
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>

#include "db_backend.h"
#include "hybfs.h"
#include "misc.h"

hybfs_db_t hybfs_db;

#define DB_ERROR(cond, message, ret) \
	if(cond) { \
		fprintf(stderr, "%s: %s\n", message, db_strerror(ret)); \
		return ret; \
	}

static int init_env()
{
	char * env_home_dir;
	int ret;
	u_int32_t env_flags;

	ret = db_env_create(&hybfs_db.core_env, 0);
	DB_ERROR((ret !=0), "Error creating env handle", -1);

	env_flags = DB_THREAD | DB_CREATE | DB_INIT_MPOOL;

	/* FIXME here we set our enviroment only in the first branch */
	env_home_dir = concat_paths(hybfs_core.branches[0].path, METADIR);
	
	ret = hybfs_db.core_env->open(hybfs_db.core_env, env_home_dir,
	                env_flags, 0);
	free(env_home_dir);
	DB_ERROR((ret !=0), "Error opening environment", -1);

	return 0;
}

static int open_db(DB_ENV *env, DB **db_out, const char *name, DBTYPE type,
                unsigned int flags)
{
	int ret;
	DB *db;

	ret = db_create(db_out, env, 0);
	DB_ERROR((ret!=0), "Error creating db handle", ret);
	db = *db_out;

	ret = db->open(db, NULL, name, NULL, DB_HASH, flags, 0);
	DB_ERROR((ret!=0), "Error opening db handle", ret);
	
	return 0;
}

int db_init_storage()
{
	int ret = 0;
	u_int32_t db_flags;

	db_flags = DB_THREAD | DB_CREATE;

	/* init the enviroment(s) */
	ret = init_env();
	if (ret != 0)
		return ret;

	/* now open the databases */
	ret = hybfs_db.main_table->set_flags(hybfs_db.main_table, DB_DUP);
	if(ret!=0) 
		goto on_error;
	ret = open_db(hybfs_db.core_env, &hybfs_db.main_table, MAINDB,
			DB_HASH, db_flags);
	if (ret != 0)
		goto on_error;

	return 0;

on_error: 
	db_close_storage();
	return 1;
}

void db_close_storage()
{
	/* close all databases here */
	if (hybfs_db.main_table != NULL)
		hybfs_db.main_table->close(hybfs_db.main_table, 0);

	/* close the enviroment(s) */
	if (hybfs_db.core_env != NULL)
		hybfs_db.core_env->close(hybfs_db.core_env, 0);
}

/*
 * Adds the file information for a tag in the main db. The tag represents
 * the key of this DB.
 */
int db_add_file_info(char *tag, file_info_t * finfo)
{
	int ret;
	DBT key, data;

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = tag;
	key.size = strlen(tag);
	data.data = finfo;
	data.size = sizeof(file_info_t);

	ret = hybfs_db.main_table->put(hybfs_db.main_table, NULL, &key, 
			&data, 0);
	DB_ERROR((ret!=0), "Error at puting new tags in DB", -1);
	
	return 0;
}

/*
 * Retreives the file information for the current position of the cursor.
 * The cursor must be opened before calling this function, and closed when
 * finish using it.
 * The data is stored in finfo.
 */
int db_get_file_info(char *tag, DBC *pcursor, int first, file_info_t *finfo)
{
	int ret, flag;
	DBT key, data;
	
	if(pcursor == NULL) {
		fprintf(stderr, "NULL cursor in function %s\n", __func__);
	}
	
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	
	key.data = tag;
	key.size = strlen(tag)+1;
	
	if(first)
		flag = DB_SET;
	else
		flag = DB_NEXT_DUP;
	
	data.data = finfo;
	data.ulen = sizeof(file_info_t);
	data.flags = DB_DBT_USERMEM;
	
	ret = pcursor->get(pcursor, &key, &data, flag);
	
	return ret;
}
