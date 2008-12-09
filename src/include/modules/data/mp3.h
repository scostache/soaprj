/* 
 mp3.h

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef MP3_H_
#define MP3_H_

#include "fileref.h"
#include "base_module.hpp"
//#include "virtualdir.hpp"

using namespace std;

class Mp3File:public GenericModule
{
private:
	TagLib::FileRef *f;
	TagLib::Tag *tag;
	char * path;

public:
	/** constructor for the mp3 module
	 * in the same time it initializes the VirtualDirectory
	 * associated with it --> const char * path
	 */
	Mp3File (const char * path);

	/** loads the tag from the file
	 * given as argument
	 * returns 0 for success, -1 for error
	 */
	virtual int load_file(const char * path);

	/** checks the file extension for module compatibility
	 * (the extension must match the modules supported file types)
	 * Returns 0 for succes, -1 for unsupported file
	 */
	virtual int check_file(const char * path);

	/** adds the file information to database */
	virtual int put_to_db();

	/** returns the type of the module */
	virtual mod_type module_type();

	/** artist information */
	string mp3_get_artist();

	/** title information */
	string mp3_get_title();

	/** album information */
	string mp3_get_album();

	/** album year information */
	string mp3_get_year();

	/** replace spaces with _ */
	string replace_spaces(string s);
};

#endif /* MP3_H_ */
