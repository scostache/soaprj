/*
 * hybfs_ops.cpp
 *
 *  Created on: Dec 2, 2008
 *      Author: dan
 */

#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <list>
#include "hybfs_ops.hpp"
#include "db_backend.hpp"
#include "hybfs.h"
#include "hybfsdef.h"

using namespace std;

HybFSOps::HybFSOps()
{

}

HybFSOps::~HybFSOps()
{
	for (vector<db_assoc>::iterator it = vect_db.begin(); it != vect_db.end(); it++) {
		delete (*it).db;
		delete (*it).path;
	}
	vect_db.clear();
}

vector<string> * HybFSOps::ops_list_db()
{
	vector<string> * ret = new vector<string>();
	for (vector<db_assoc>::iterator it = vect_db.begin(); it != vect_db.end(); ++it) {
		ret->push_back((*it).path);
	}
	return ret;
}

int HybFSOps::ops_load_db(const char *path)
{
	/* verify if there is already a database with the path */
	if(!verify_database(path))
		return -1;

	db_assoc da;
	DbBackend *db;
	struct stat buf;
	string abspath = path;
	abspath.append(METADIR);
	db = NULL;

	/* check if the directory exists, if not - create */
	if (lstat(abspath.c_str(), &buf) == -1) {
		DBG_PRINT("Error at checking directory! Atempt to create one!\n");
		/* TODO get appropriate permisions for our directory */
		if (mkdir(abspath.c_str(), 0755)) {
			perror("Failed to create directory: ");
		}
	}
	abspath.append(MAINDB);

	db = new DbBackend(abspath.c_str());
	da.path = strdup(path);
	da.db = db;
	vect_db.push_back(da);

	return db->db_init_storage();
}

int HybFSOps::verify_database(const char *path)
{
	for (vector<db_assoc>::iterator it = vect_db.begin(); it != vect_db.end(); it++) {
			if (strcmp((*it).path, path) == 0)
				return 0;
		}
	return -1;
}

list<string> * HybFSOps::ops_list_tags(const char *path)
{
	DbBackend *db = get_database(path);
	if (db == NULL)
		return NULL;

	return db->db_get_tags_values(NULL);
}

DbBackend * HybFSOps::get_database(const char *path)
{
	for (vector<db_assoc>::iterator it = vect_db.begin(); it != vect_db.end(); ++it) {
		if (strncmp((*it).path, path, strlen((*it).path)) == 0)
			return (*it).db;
	}
	return NULL;
}

const char * HybFSOps::get_vdir_path(const char *path)
{
	for (vector<db_assoc>::iterator it = vect_db.begin(); it != vect_db.end(); it++) {
		if (strncmp((*it).path, path, strlen((*it).path)) == 0)
			return (*it).path;
	}
	return NULL;
}

list<string> * HybFSOps::ops_list_file_tags(const char *path)
{
	DbBackend *db = get_database(path);
	if (db == NULL)
		return NULL;

	return db->db_get_tags_values(path + strlen(get_vdir_path(path)));
}

const char * HybFSOps::get_tag(const char *str)
{
	char *ret;
	char *str_copy = strdup (str);
	char *delim = strrchr (str_copy, ':');
	(*delim) = '\0';
	ret = (char *) malloc ((delim - str_copy + 1) * sizeof (char));
	strcpy(ret, str_copy);

	delete str_copy;
	return ret;
}

const char * HybFSOps::get_value(const char *str)
{
	char *delim = strrchr(str, ':');
	return strdup (delim + 1);
}



int HybFSOps::ops_file_add_tag(const char *tag, const char *path)
{
	int ret = 0;
	DbBackend *db = get_database(path);




	return ret;
}

int HybFSOps::ops_file_remove_tag(const char *tagvalue, const char *path)
{
	int ret = 0;
	DbBackend *db = get_database(path);
	if (db == NULL)
		return -1;
	const char * tag = this->get_tag(tagvalue);
	const char * value = this->get_value(tagvalue);

	cout<<tag<<":"<<value<<endl;
	ret = db->db_delete_file_tag(tag, value, path + strlen(get_vdir_path(path)));

	if (tag)
		delete tag;
	if (value)
		delete value;

	return ret;
}

file_info_t * HybFSOps::get_file_info(const char * path)
{
	struct stat *buf;			// file information structure for stat function
	file_info_t *finfo = NULL;	// structure with info to populate the database

	buf = (struct stat *) malloc (sizeof(struct stat));
	if (!buf)
		return NULL;

	/* if the stat function returns an error */
	if (0 != stat(path, buf)) {
		return NULL;
	}

	/* strip the path in order to be relative
	 * to the virtual dir
	 */
	const char * vdir_path = get_vdir_path(path);

	finfo = (file_info_t *) malloc (sizeof(file_info_t) + strlen(path) + 1 - strlen(vdir_path));
	if (!finfo) {
		free (buf);
		return NULL;
	}

	/* copy stat information in the finfo structure */
	finfo->fid = buf->st_ino;
	finfo->mode = buf->st_mode;
	finfo->namelen = strlen(path) - strlen(vdir_path);
	strcpy(finfo->name, path + strlen(vdir_path));

	/* clean up the mess */
	delete (buf);

	return finfo;
}

int HybFSOps::ops_copy_file(const char *src, const char *dst)
{
	DbBackend *src_db, *dst_db;

	src_db = get_database(src);
	dst_db = get_database(dst);

	if (src_db == NULL || dst_db == NULL)
		return -1;

	/* verify if the path and the file exists */
	struct stat buf;
	if (stat(src, &buf) != 0)
		return -1;

	char *dst_copy = strdup(dst);
	char *delimiter;
	delimiter = strrchr (dst_copy, '/');
	(*delimiter) = '\0';
	if (lstat(dst_copy, &buf) != 0) {
		delete dst_copy;
		return -1;
	}
	delete dst_copy;

	/* copy the file effectively */
	ifstream inputFile (src, ifstream::in);
	ofstream outputFile (dst);

	outputFile << inputFile.rdbuf();


	/* get file tags */
	list<string> *tags = src_db->db_get_tags_values(src + strlen(get_vdir_path(src)));

	/* the file have no tags -->
	 * TODO: if path/filename is saved in the database
	 * this data will be copied
	 */
	if (tags == NULL) {
		return 0;
	}

	/* get the new file attributes ... to populate the new database */
	/* TODO: WHO frees the memory ???? */
	vector<string> vtags; // = new vector<string>();
	file_info_t *finfo;		// structure with info to populate the database

	/* extract inode information about the destination file */
	finfo = get_file_info(dst);

	if (!finfo) {
		return -1;
	}

	/* copy the tags list into the vtags vector */
	for (list<string>::iterator it = tags->begin(); it != tags->end(); ++it) {
		vtags.push_back((*it));
	}

	cout<<"vectorul contine:\n";
	for (vector<string>::iterator it = vtags.begin(); it != vtags.end(); ++it)
			cout<<(*it)<<endl;
	/* set the tags for the file in the destination database */
	dst_db->db_add_file_info(&vtags, finfo);

	/* memory clean */
	delete (finfo);
	if (tags) {
		tags->clear();
		delete tags;
	}
	if (vtags.empty()) {
		vtags.clear();
		//delete vtags; I believe this is done at function exit
	}





	/* TODO: if the dst file exists ... delete the apropriate entries in the database */
	return 0;
}

