/*
 * module_loader.cpp
 *
 *  Created on: Dec 3, 2008
 *      Author: dan
 */

#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdlib>
#include "module_loader.hpp"
#include "base_module.hpp"
#include "mp3.h"
#include "pict.hpp"

using namespace std;

ModuleManager::ModuleManager()
{

}
ModuleManager::~ModuleManager()
{
	/* free memory */

}

string ModuleManager::mod_get_type(mod_type t)
{
	string s;
	switch (t) {
	case MP3: s = "MP3"; break;
	case EXIF: s = "EXIF"; break;
	case MOD_ERROR: s = "unknown"; break;
	}

	return s;
}

/** add a new module for a specific path
 * Returns 0 on SUCCESS, -1 on ERROR
 */
int ModuleManager::mod_add_module (mod_type t, const char * path)
{
	printf ("mod_add_module(): %s\n", path);
	int ret = 0;
	module_assoc ma;

	switch(t)
	{
	case MP3:
		ma.module = new Mp3File(path);
		break;
	case EXIF:
		ma.module = new PictFile(path);
		break;
	case MOD_ERROR:
		return -1;
		break;
	}
	ma.path = strdup (path);

	/* add the new module */
	modules.push_back (ma);

	return ret;
}

vector<string>* ModuleManager::mod_list_modules()
{
	vector<string> *ret = new vector<string>();
	for (vector<module_assoc>::iterator i = this->modules.begin(); i != this->modules.end(); i++) {
		string s = (*i).path;
		s.append(" ");
		s.append(mod_get_type((*i).module->module_type()));
		ret->push_back(s);
	}
	return ret;
}

/** removes the module of type "t" for the
 * path "path".
 * Returns 0 on SUCCESS, -1 on ERROR
 */
int ModuleManager::mod_remove_module (mod_type t, const char * path)
{
	for (vector<module_assoc>::iterator i = this->modules.begin(); i != this->modules.end(); i++) {
		if (strcmp((*i).path, path) == 0 && (*i).module->module_type() == t) {
			/* free the memory used for the module */
			delete ((*i).module);
			delete ((*i).path);
			this->modules.erase(i);
		}
	}
	return -1;
}

/** verifies if there is a module of type "t" in the specified path */
int ModuleManager::mod_verify_module (mod_type t, const char * path)
{
	for (vector<module_assoc>::iterator i = this->modules.begin(); i != this->modules.end(); i++) {
		if (strcmp((*i).path, path) == 0)
			if ((*i).module->module_type() == t)
				return 0;
	}
	return -1;
}

/** process a file and add it to the apropriate database */
int ModuleManager::mod_process_file (const char * path)
{
	int ret = -1;

	printf ("Processing ... %s \n", path);
	/** search for the appropriate module to process the file */
	for (vector<module_assoc>::iterator i = this->modules.begin(); i != this->modules.end(); i++) {
		if (strncmp((*i).path, path, strlen((*i).path)) == 0) {
			if (((*i).module->check_file(path)) == 0) {
				printf ("found the module\n");
				(*i).module->load_file(path);
				printf ("loaded the file\n");
				(*i).module->put_to_db();
				ret = 0;
			}
		}
	}

	return ret;
}

mod_type ModuleManager::mod_char_to_type(char * type)
{
	if (strcmp(type, "mp3") == 0)
		return MP3;
	else if (strcmp(type, "exif") == 0)
		return EXIF;

	return MOD_ERROR;
}

int ModuleManager::mod_read_conf_file (FILE * f)
{
	size_t sz;
	char * ptr;
	char ** line;
	line = (char **) malloc (sizeof (char *));
	//count = fscanf(f, "%s %s %s", unu, doi, trei);
	while(getline(line, &sz, f) != -1) {
		char *aux = strdup (*line);
		ptr = aux;
		while((*ptr) != '\n' && ptr < (aux + sz))
			ptr++;
		(*ptr) = '\0';
		ptr = aux;
		while((*ptr) != ' ' && ptr <(aux + sz))
			ptr++;
		(*ptr) = '\0';
		ptr++;

		this->mod_add_module(mod_char_to_type(aux), ptr);

		if (aux != NULL)
			delete aux;
	}
	return 0;
}
