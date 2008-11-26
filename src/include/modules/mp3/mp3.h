/*
 * mp3.h
 *
 *  Created on: Nov 25, 2008
 *      Author: dan
 */

#ifndef MP3_H_
#define MP3_H_



#endif /* MP3_H_ */

#include "fileref.h"
#include "db_backend.hpp"
using namespace std;

class Mp3File
{
private:
	TagLib::FileRef *f;
	TagLib::Tag *tag;
	DbBackend *db;

public:
	/** default constructor */
	Mp3File ();

	/** constructor and file load in the same time */
	Mp3File (char * file);

	/** loads the tag from the file
	 * given as argument
	 * returns 0 for success, -1 for error
	 */
	int mp3_load_file(char * file);

	/** artist information */
	string mp3_get_artist();

	/** title information */
	string mp3_get_title();

	/** album information */
	string mp3_get_album();

	/** album year information */
	uint mp3_get_year();

	/** replace spaces with _ */
	string replace_spaces(string s);

	/** selects the database path */
	void db_init(const char * path);

	/** inserts data into the database */
	int mp3_db_insert_tag();

	/** closes the database */
	void db_close();

};
