/*
 mp3.cpp - Extracts tags from mp3 files and adds them in the DB.

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <iostream>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileref.h"
#include "tstring.h"
#include "tag.h"
#include "mp3.h"
#include "base_module.hpp"
#include "core/virtualdir.hpp"

using namespace std;

using namespace hybfs;

TagLib::String formatSeconds(int seconds)
{
  char secondsString[3];
  sprintf(secondsString, "%02i", seconds);
  return secondsString;
}


Mp3File::Mp3File(const char * path)
{
	this->f		= NULL;
	this->tag	= NULL;

	/* initializes the vdir path for the database */
	init_vdir (path);
}


int Mp3File::load_file(const char * path)
{
	delete(f);
	this->f = new TagLib::FileRef(path);
	if (!f->isNull() && f->tag()) {
		this->tag = f->tag();
		return 0;
	}
	else {
		this->tag = NULL;
		return -1;
	}
}

mod_type Mp3File::module_type()
{
	return MP3;
}


int Mp3File::check_file(const char * path)
{
	char ext[4];

	strncpy(ext, path + (strlen(path)-3), 3);
	ext[3] = '\0';
	for (int i = 0; i < 3; i++)
		ext[i] = tolower(ext[i]);

	return (strcmp(ext, "mp3") == 0) ? 0 : -1;
}

string Mp3File::mp3_get_artist()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return (tag->artist()).to8Bit();
	return NULL;
}

string Mp3File::mp3_get_album()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return (tag->album()).to8Bit();
	return NULL;
}

string Mp3File::mp3_get_title()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return (tag->title()).to8Bit();
	return NULL;
}

string Mp3File::mp3_get_year()
{
	char buf[10];
	if (tag != NULL) {
		sprintf (buf, "%u", tag->year());
		string s(buf);
		return s;
	}
	return NULL;
}

int Mp3File::put_to_db()
{
	int ret = 0;			// return value
	vector<string> *tags = new vector<string>();
	file_info_t *finfo;		// structure with info to populate the database

	/* extract inode information about the file */
	finfo = get_file_info(this->f->file()->name());

	if (!finfo) {
		return -1;
	}

	if (!mp3_get_album().empty())
		tags->push_back(string("album:" + replace_spaces(mp3_get_album())));
	if (!mp3_get_artist().empty())
		tags->push_back(string("artist:" + replace_spaces(mp3_get_artist())));
	if (!mp3_get_title().empty())
		tags->push_back(string("title:" + replace_spaces(mp3_get_title())));
	if (!mp3_get_year().empty())
		tags->push_back(string("year:" + mp3_get_year()));


	int res = vdir->update_file (tags, TAG_ADD, finfo, 0);
	/* TODO: !!!!!!!!!!!!!! .... I have to use RES !!!!!!!!!!!!!!!!!! */

	/* memory clean */
	delete (finfo);
	tags->clear();

	return ret;
}


string Mp3File::replace_spaces(string s)
{
	string ret = s;

	for (unsigned int i = 0; i < ret.size(); i++) {
		if (ret[i] == ' ') ret[i]='_';
	}

	return ret;
}
