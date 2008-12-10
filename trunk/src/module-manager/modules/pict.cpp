/*
 pict.cpp- Extracts EXIF info and puts it in the DB

 Copyright (C) 2008-2009  Dan Pintilei

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <cstring>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <iomanip>
#include <iostream>

#include "image.hpp"
#include "exif.hpp"
#include "pict.hpp"
#include "base_module.hpp"
#include "core/virtualdir.hpp"

using namespace std;

PictFile::PictFile(const char * path)
{
	file_path = NULL;
	init_vdir(path);
}

PictFile::~PictFile ()
{
	if (file_path)
		delete file_path;
}

string PictFile::erase_end_spaces(string s)
{
	string str = s;
	for (string::iterator it = str.end() - 1; it != str.begin();) {
		if ((*it) == ' ')
			it = str.erase(it) - 1;
		else
			break;
	}
	return str;
}


mod_type PictFile::module_type()
{
	return EXIF;
}

/** loads the exif information from file
 * returns 0 for success, -1 for error
 */
int PictFile::load_file(const char * path)
{
	printf ("file path begin\n");
	if (file_path) {
		printf ("in if\n");
		delete file_path;
	}
	file_path = strdup(path);
	if (file_path == NULL)
		return -1;
	return 0;
}

/** checks the file extension for module compatibility
 * (the extension must match the modules supported file types)
 * Returns 0 for succes, -1 for unsupported file
 */
int PictFile::check_file(const char * path)
{
	char ext[5];

	strncpy(ext, path + (strlen(path)-4), 4);
	ext[4] = '\0';

	for (int i = 0; i < 4; i++)
			ext[i] = tolower(ext[i]);

	return (strcmp(ext, ".jpg") == 0 || strcmp(ext, "jpeg") == 0 || strcmp(ext, "tiff") == 0) ? 0 : -1;
}

string PictFile::get_month(string str)
{
	string s = str.substr(5, 2);
	int month = atoi(s.c_str());
	string ret;
	switch (month) {
		case 1: ret = "january"; break;
		case 2: ret = "february"; break;
		case 3: ret = "march"; break;
		case 4: ret = "april"; break;
		case 5: ret = "may"; break;
		case 6: ret = "june"; break;
		case 7: ret = "july"; break;
		case 8: ret = "august"; break;
		case 9: ret = "september"; break;
		case 10: ret = "october"; break;
		case 11: ret = "november"; break;
		case 12: ret = "december"; break;
		default: ret = "unknown";
	}
	return ret;
}

string PictFile::get_year(string str)
{
	return str.substr(0, 4);
}

string PictFile::replace_spaces(string s)
{
	string ret = s;

	for (unsigned int i = 0; i < ret.size(); i++) {
		if (ret[i] == ' ') ret[i]='_';
	}

	return ret;
}

/** adds the file information to database */
int PictFile::put_to_db()
{
	int ret = 0;			// return value
	vector<string> *tags = new vector<string>();
	file_info_t *finfo;		// structure with info to populate the database

	if (!file_path)
		return -1;

	/* extract inode information about the file */
	finfo = get_file_info(this->file_path);

	if (!finfo) {
		return -1;
	}

	try {
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file_path);
		assert(image.get() != 0);
		image->readMetadata();

		Exiv2::ExifData &exifData = image->exifData();
		if (exifData.empty()) {
			string error(this->file_path);
			error+= ": No Exif data found in the file";
			throw Exiv2::Error(1, error);
		}

		printf ("before tags\n");

		int count = 0;
		tags->push_back(string("type:image"));
		Exiv2::ExifData::const_iterator end = exifData.end();
		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; i++) {
			if (strcmp(i->key().c_str(), "Exif.Image.ImageDescription") == 0) {
				string s = erase_end_spaces("description:" + i->value().toString());
				tags->push_back(replace_spaces(s));
				count++;
			}
			else if (strcmp(i->key().c_str(), "Exif.Image.Model") == 0) {
				string s = erase_end_spaces("camera:" + i->value().toString());
				tags->push_back(replace_spaces(s));
				count++;
			}
			else if (strcmp(i->key().c_str(), "Exif.Image.DateTime") == 0) {
				tags->push_back("year:" + get_year(i->value().toString()));
				string month = get_month(i->value().toString());
				if (month.compare("unknown") != 0)
					tags->push_back("month:" + month);
				count++;
			}
			if (count == 3)
				break;
		}
	}
	catch(Exiv2::AnyError& e) {
		printf ("error\n");
		tags->clear();
		delete tags;
		return -1;
	}

	int res = vdir->update_file(tags, TAG_ADD, finfo, 0);

	/* memory clean */
	delete (finfo);
	tags->clear();

	return ret;
}
