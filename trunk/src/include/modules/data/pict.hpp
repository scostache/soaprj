/* 
pict.hpp

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef PICT_HPP_
#define PICT_HPP_

#include <cstdio>
#include "base_module.hpp"

using namespace std;

class PictFile:public GenericModule
{
private:
	char * file_path;

public:
	/** constructor for the exif module
	 * in the same time it initializes the VirtualDirectory
	 * associated with it --> const char * path
	 */
	PictFile (const char * path);

	~PictFile();

	/** returns the type of the module */
	virtual mod_type module_type();

	/** loads the exif information from file
	 * returns 0 for success, -1 for error
	 */
	virtual int load_file (const char * path);

	/** checks the file extension for module compatibility
	 * (the extension must match the modules supported file types)
	 * Returns 0 for succes, -1 for unsupported file
	 */
	virtual int check_file (const char * path);

	/** adds the file information to database */
	virtual int put_to_db ();

	/** erases all the spaces at the end of the string */
	string erase_end_spaces(string s);

	/** extracts the month out of DateTime variable from EXIF info */
	string get_month(string str);

	/** extracts the year out of DateTime variable from EXIF info */
	string get_year(string str);

	/** replaces the spaces with "_" in order to follow
	 * the tag rules (without spaces)
	 */
	string replace_spaces(string s);
};

#endif /* EXIF_HPP_ */
