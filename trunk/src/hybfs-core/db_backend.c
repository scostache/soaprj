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
#include "hybfsdef.h"
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

	DBG_SHOWFC();
	
	ret = db_env_create(&hybfs_db.core_env, 0);
	DB_ERROR((ret !=0), "Error creating env handle", -1);

	env_flags = DB_THREAD | DB_CREATE | DB_INIT_MPOOL;

	/* FIXME here we set our enviroment only in the first branch */
	env_home_dir = concat_paths(hybfs_core.branches[0].path, METADIR);
	fprintf(stderr, "env path is: %s \n", env_home_dir);
	
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

	DBG_SHOWFC
	();

	ret = db_create(db_out, env, 0);
	DB_ERROR((ret!=0), "Error creating db handle", ret);
	db = *db_out;

	if (flags & DB_DUP) {
		ret = hybfs_db.main_table->set_flags(hybfs_db.main_table,
		                DB_DUP);
		DB_ERROR((ret!=0), "Error seting flags tp db handle", ret);
	}
	ret = db->open(db, NULL, name, NULL, DB_HASH, flags, 0);
	DB_ERROR((ret!=0), "Error opening db handle", ret);

	return 0;
}

int db_init_storage()
{
	int ret = 0;
	u_int32_t db_flags;

	DBG_SHOWFC();
	
	db_flags = DB_THREAD | DB_CREATE;

	/* init the enviroment(s) */
	ret = init_env();
	if (ret != 0)
		return ret;

	/* now open the databases */
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
	DBG_SHOWFC();
	
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
	
	DBG_SHOWFC();

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = tag;
	key.size = strlen(tag)+1;
	data.data = finfo;
	data.size = sizeof(file_info_t) + finfo->namelen + 1;

	/* Don't allow duplicate data with the same key */
	ret = hybfs_db.main_table->put(hybfs_db.main_table, NULL, &key, 
			&data, DB_NODUPDATA);
	DB_ERROR((ret!=0), "Error at puting new tags in DB", -1);
	
	return 0;
}

/*
 * Retreives the file information for the current position of the cursor.
 * The cursor must be opened before calling this function, and closed when
 * finish using it.
 * The data is stored in finfo.
 */
int db_get_file_info(char *tag, DBC *pcursor, int first, file_info_t **finfo)
{
	int ret, flag;
	DBT key, data;
	
	DBG_SHOWFC();
	
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
	
	ret = pcursor->get(pcursor, &key, &data, flag);
	*finfo = (file_info_t *) data.data;
	
	return ret;
}

/*
 * Deletes all the records from the main DB, coresponding
 * to the key "tag"
 */
int db_delete_allfile_info(char *tag)
{
	DBT key;
	int ret;
	
	memset(&key, 0, sizeof(DBT));
	key.data = tag;
	key.size = strlen(tag);
	ret = hybfs_db.main_table->del(hybfs_db.main_table, NULL, &key, 0);
	DB_ERROR((ret!=0), "Error at deleting a tag from DB", -1);
	
	return 0;
}

/*
 * Deletes the record with the absolute path "abspath"
 * coresponding to the key "tag"
 */
int db_delete_file_info(char *tag, char *abspath)
{

	return 0;
}

/* 
 * Checks if the tag is really a key in the main DB.
 * Returns 1 if it exists, 0 otherwise.
 */
int db_check_tag(char *tag)
{
	DBT key, data;
	int res = 0;

	/* suppose I try to do a get with the key "tag" */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = tag;
	key.size = strlen(tag)+1;

	res = hybfs_db.main_table->get(hybfs_db.main_table, NULL, &key, 
					&data, 0);
	if (res == 0)
		return 1;

	return 0;
}
