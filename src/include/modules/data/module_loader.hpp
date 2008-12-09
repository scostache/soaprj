/*
 * module_loader.hpp
 *
 *  Created on: Dec 2, 2008
 *      Author: dan
 */

#ifndef MODULE_LOADER_HPP_
#define MODULE_LOADER_HPP_

#include "base_module.hpp"
#include <vector>

using namespace std;

/** structure used to associate a specific module
 * with a path (the path of its VirtualDirectory)
 */
struct module_assoc
{
	GenericModule *module;
	const char * path;
};

class ModuleManager
{
private:
	vector<module_assoc> modules;
public:
	ModuleManager();
	~ModuleManager();

	/** add a new module for a specific path
	 * Returns 0 on SUCCESS, -1 on ERROR
	 */
	int mod_add_module (mod_type t, const char * path);

	/** removes the module of type "t" for the
	 * path "path".
	 * Returns 0 on SUCCESS, -1 on ERROR
	 */
	int mod_remove_module (mod_type t, const char * path);

	/** verifies if there is a module of type "t" in the specified path */
	int mod_verify_module (mod_type t, const char * path);

	/** process a file and add it to the apropriate database */
	int mod_process_file (const char * path);

	/** lists all the modules loaded */
	vector<string> * mod_list_modules();

	/** mod_type to string converter */
	string mod_get_type(mod_type t);

	/** conversion from char* to mod_type */
	mod_type mod_char_to_type(char * type);

	/** reads an configuration file to automatically
	 * load the appropriate vdir paths
	 */
	int mod_read_conf_file (FILE *);
};

#endif /* MODULE_LOADER_HPP_ */
