/*
 module_manager.cpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

#include <limits.h>
#include <fnmatch.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <list>

#include "mp3.h"
#include "pict.hpp"
#include "hybfs_ops.hpp"
#include "base_module.hpp"
#include "module_loader.hpp"

#define FILTER1 "*.[mM][pP]3"
#define FILTER2 "*.[jJ][pP][gG]"

using namespace std;

int scandirectory(const char *dirname, ModuleManager *m)
{
	DIR *dir;
	struct dirent *entry;
	char path[PATH_MAX];

	if (path == NULL) {
		fprintf(stderr, "Out of memory error\n");
		return 0;
	}
	dir = opendir(dirname);
	if (dir == NULL) {
		perror("Error opendir()");
		return 0;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".")
					&& strcmp(entry->d_name, "..")) {
				snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname,
						entry->d_name);
				scandirectory(path, m);
			}
		} else if (entry->d_type == DT_REG) {
			if (!fnmatch(FILTER1, entry->d_name, 0) || !fnmatch(FILTER2, entry->d_name, 0)) {
				/* files that are matched are passed to the ModuleManager
				 * object to index them apropriately
				 */
				char *full_path;
				full_path = (char *) malloc ((strlen(dirname) + strlen(entry->d_name) + 2) * sizeof(char));
				sprintf (full_path, "%s/%s", dirname, entry->d_name);
				m->mod_process_file(full_path);
				if (full_path != NULL)
					free (full_path);
			}
		}
	}
	closedir(dir);
	return 1;
}


void print_help()
{
	cout<<"module_manager usage:"<<endl<<endl;
	cout<<"help - help information\n\n";
	cout<<"ls /dir_path/"<<endl;
	cout<< "\t-list all tags found in the /dir_path/ location if this location is a mount point\n\n";
	cout<<"ls /file_path\n\t-list all tags for the file /file_path with the condition that the file is located somewhere in the path of a mounted directory\n\n";
	cout<<"list [path | modules]\n";
	cout<<"\t-the list of all modules loaded [modules] or mounted directories paths [path]\n\n";
	cout<<"parse file_path\n\t-if there is a module loaded for the path where the file resides, the information extracted from the file will be loaded in the appropriate database\n\n";
	cout<<"parsedir dir_path\n\t-searches recursively in the dir_path and parses all the files that have the apropriate extensions\n\n";
	cout<<"cp src_location dst_location\n\t"<<"-copies a file along with its tags from one location to another. If the dst_location is in another mounted directory... the tag information will be stored in the appropriate database\n\n";
}


int main(int argc, char** argv)
{
	ModuleManager m;
	HybFSOps ops;

	/* trying to load the default module configuration */
	FILE * f_conf;

	/* open the default configuration file */
	f_conf = fopen(".default_conf", "r");
	if (f_conf) {
		/* reading the default configuration file */
		m.mod_read_conf_file (f_conf);
		fclose (f_conf);

	}

	/* try to open the file for writing */
	/*f_conf = fopen(".default_conf", "a+");
	if (!f_conf) {
		printf ("ERROR: could not create configuration file...\n");
		return -1;
	}
	*/

	/* load default hyb_ops configuration */
	FILE * ops_conf;

	/* open the default configuration file */
	ops_conf = fopen(".default_ops_conf", "r");
	if (ops_conf) {
		/* reading the default configuration file */
		ops.ops_read_conf_file (ops_conf);
		fclose (ops_conf);
	}



	string s;
	int ret;
	int count;
	char cmd[100], arg1[100], arg2[100];

	while (1) {
		cout<<"command: ";
		getline(cin, s);
		count = sscanf(s.c_str(), "%s %s %s", cmd, arg1, arg2);

		if (strcmp(cmd, "exit") == 0)
			break;
		if (strcmp(cmd, "help") == 0)
			print_help();

		if (count == 2) {
			if (strcmp(cmd, "parse") == 0) {
				m.mod_process_file(arg1);
			}
			else if (strcmp(cmd, "parsedir") == 0) {
				scandirectory(arg1, &m);
			}
			else if (strcmp(cmd, "path") == 0) {
				ret = ops.ops_load_db(arg1);
				if (ret == -1)
					printf ("can't load databse for directory %s\n", arg1);
				else
					printf ("database for dir %s loaded\n", arg1);
			}
			else if (strcmp(cmd, "list") == 0) {
				if (strcmp(arg1, "path") == 0) {
					vector<string> *db_list = ops.ops_list_db();
					cout<<"listing vdirs ..."<<endl;
					count = 1;
					for (vector<string>::iterator i = db_list->begin();
					i != db_list->end(); ++i) {
						cout<<count<<". "<<(*i)<<endl;
						count++;
					}
					db_list->clear();
					delete db_list;
				}
				else if (strcmp(arg1, "modules") == 0) {
					vector<string> *modules_list = m.mod_list_modules();
					cout<<"listing modules ..."<<endl;
					count = 1;
					for (vector<string>::iterator i = modules_list->begin();
					i != modules_list->end(); ++i) {
						cout<<count<<". "<<(*i)<<endl;
						count++;
					}
					modules_list->clear();
					delete modules_list;
				}
			}
			else if (strcmp(cmd, "ls") == 0) {
				list<string> *lst;
				const char *aux = ops.get_vdir_path(arg1);
				if (aux == NULL) {
					cout<<"invalid path \""<<arg1<<"\"\n";
					continue;
				}
				if (strlen(aux) == strlen(arg1)) {
					lst = ops.ops_list_tags(arg1);
				}
				else {
					lst = ops.ops_list_file_tags(arg1);
				}
				cout<<arg1<<" tags:";
				if (lst->empty())
					cout<<" ... no tags"<<endl;
				else
					cout<<endl;

				for (list<string>::iterator it = lst->begin(); it != lst->end(); ++it) {
					cout<<(*it)<<endl;
				}

				lst->clear();
				delete lst;
			}
		}
		else if (count == 3) {
			if (strcmp(cmd, "cp") == 0) {
				ops.ops_copy_file (arg1, arg2);
			}
			else if (strcmp(cmd, "rm") == 0) {
				if (ops.ops_file_remove_tag(arg1, arg2) == 0) {
					cout<<"tag \""<<arg1<<"\" removed\n";
				}
				else {
					cout<<"error removing tag \""<<arg1<<endl;
				}

			}
			else if (strcmp(cmd, "module") == 0) {
				int ret = m.mod_add_module(m.mod_char_to_type(arg1), arg2);
				if (ret != 0)
					cout<<"loading failed !!!\n";
				else
					cout<<"loading "<<arg2<<" "<<arg1<<" module ... ok"<<endl;
			}
		}

	}


}


