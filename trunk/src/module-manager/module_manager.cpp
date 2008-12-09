/* 
 module_manager.cpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <cstdio>
#include <iostream>
#include <list>
#include <cstdlib>
#include <cstring>
#include "base_module.hpp"
#include "mp3.h"
#include "pict.hpp"
#include "hybfs_ops.hpp"
#include "module_loader.hpp"

using namespace std;


int main(int argc, char** argv)
{
	ModuleManager m;
	HybFSOps ops;

	/* trying to load the default configuration */
	FILE * f_conf;

	/* open the default configuration file */
	f_conf = fopen(".default_conf", "r");
	if (!f_conf) {
		/* try to create the file */
		f_conf = fopen(".default_conf", "w");
		if (!f_conf) {
			printf ("ERROR: could not create configuration file...\n");
			return -1;
		}
	}
	else {
		/* reading the default configuration file */
		m.mod_read_conf_file (f_conf);
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

		if (count == 2) {
			if (strcmp(cmd, "parse") == 0) {
				m.mod_process_file(arg1);
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
		}

	}

//	char command[100], arg1[100], arg2[100];
//	int count = 0;
//	string s;
//	while (1) {
//		cout<<"command: ";
//		getline(cin, s);
//		count = sscanf(s.c_str(), "%s %s %s", command, arg1, arg2);
//		cout<<count<<": "<<command<< " | "<< arg1 << " | "<< arg2<<endl;
//		if (strcmp(command, "exit") == 0)
//			break;
//
//		if (count == 2) {
//			cout<<"hello";
//		}
//	}

}


