/* 
 base_module.hpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef BASE_MODULE_HPP_
#define BASE_MODULE_HPP_

#include "core/hybfsdef.h"
#include "core/virtualdir.hpp"

using namespace hybfs;

enum mod_type
{
	MP3,
	EXIF,
	MOD_ERROR
};

class GenericModule
{
private:
	char * path;
public:
	VirtualDirectory *vdir;	// vdir associated with the module

	/** constructor */
	GenericModule();

	/** destructor */
	~GenericModule();

	/** initializes the virtual directory
	 * used by the implemented module
	 */
	void init_vdir(const char * path);

	/** helping function comon to all modules that gets
	 * the file information using "stat" function
	 */
	file_info_t * get_file_info(const char * path);

	/** returns the type of the module */
	virtual mod_type module_type() = 0;

	/** virtual function that will be implemented by
	 * all classes that will define this virtual class
	 * This will load information specific
	 */
	virtual int load_file (const char * path) = 0;

	/** checks the extension of the file that
	 * corresponds with the specific module that
	 * implemented this class
	 */
	virtual int check_file (const char * path) = 0;

	/** puts module specific information in the database
	 */
	virtual int put_to_db () = 0;

};


#endif /* BASE_MODULE_HPP_ */
