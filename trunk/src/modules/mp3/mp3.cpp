//============================================================================
// Name        : mp3.cpp
// Author      : Dan P.
// Version     : 1.0
// Copyright   : Your copyright notice
// Description : Extracts tags from mp3 files and adds them in the DB.
//============================================================================


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
#include "db_backend.hpp"

using namespace std;

TagLib::String formatSeconds(int seconds)
{
  char secondsString[3];
  sprintf(secondsString, "%02i", seconds);
  return secondsString;
}


Mp3File::Mp3File(char * file)
{
	this->f = new TagLib::FileRef(file);

	if (!f->isNull() && f->tag()) {
		this->tag = f->tag();
	}
	else
		this->tag = NULL;
}

int Mp3File::mp3_load_file(char * file)
{
	delete(f);
	this->f = new TagLib::FileRef(file);
	if (!f->isNull() && f->tag()) {
		this->tag = f->tag();
		return 0;
	}
	return -1;
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

uint Mp3File::mp3_get_year()
{
	if (tag != NULL)
		return tag->year();
	return NULL;
}

/** selects the database path */
void Mp3File::db_init(const char * path)
{
	this->db = new DbBackend(path);

	/* initialize the storage */
	if ((this->db->db_init_storage()) != 0) {
		this->db = NULL;
	}
}

	/** inserts data into the database */
int Mp3File::mp3_db_insert_tag()
{
	int ret = 0;
	vector<string> *tags = new vector<string>();
	file_info_t *finfo;


	finfo = (file_info_t *) malloc (sizeof(file_info_t));

	if (!mp3_get_album().empty())
		tags->push_back(string("album:" + replace_spaces(mp3_get_album())));
	if (!mp3_get_artist().empty())
		tags->push_back(string("artist:" + replace_spaces(mp3_get_artist())));
	if (!mp3_get_title().empty())
		tags->push_back(string("title:" + replace_spaces(mp3_get_title())));
	if (mp3_get_year() != NULL)
		tags->push_back(string("year:" + mp3_get_year()));

	if (f == NULL) {
		ret = -1;

		/* free memory */
		delete (tags);

		return ret;
	}

	/* extract the inode, mode properties for the file */
	struct stat *buf;

	buf = (struct stat *) malloc (sizeof(struct stat));
	if (!stat(this->f->file()->name(), buf)) {
		delete (tags);
		return -1;
	}

	finfo->fid = buf->st_ino;
	finfo->mode = buf->st_mode;
	finfo->namelen = strlen(this->f->file()->name());
	printf ("%s file_name\n", this->f->file()->name());
	strcpy(finfo->name, this->f->file()->name());

	delete (buf);

	this->db->db_add_file_info(tags, finfo);


	return ret;
}

/** closes the database */
void Mp3File::db_close()
{
	if (db != NULL)
		this->db->db_close_storage();
}


string Mp3File::replace_spaces(string s)
{
	string ret = s;

	for (unsigned int i = 0; i < ret.size(); i++) {
		if (ret[i] == ' ') ret[i]='_';
	}

	return ret;
}



/*
int main(int argc, char *argv[])
{
  for(int i = 1; i < argc; i++) {

    cout << "******************** \"" << argv[i] << "\" ********************" << endl;

    TagLib::FileRef f(argv[i]);

    if(!f.isNull() && f.tag()) {

      TagLib::Tag *tag = f.tag();

      cout << "-- TAG --" << endl;
      cout << "title   - \"" << tag->title()   << "\"" << endl;
      cout << "artist  - \"" << tag->artist()  << "\"" << endl;
      cout << "album   - \"" << tag->album()   << "\"" << endl;
      cout << "year    - \"" << tag->year()    << "\"" << endl;
      cout << "comment - \"" << tag->comment() << "\"" << endl;
      cout << "track   - \"" << tag->track()   << "\"" << endl;
      cout << "genre   - \"" << tag->genre()   << "\"" << endl;
    }

    if(!f.isNull() && f.audioProperties()) {

      TagLib::AudioProperties *properties = f.audioProperties();

      int seconds = properties->length() % 60;
      int minutes = (properties->length() - seconds) / 60;

      cout << "-- AUDIO --" << endl;
      cout << "bitrate     - " << properties->bitrate() << endl;
      cout << "sample rate - " << properties->sampleRate() << endl;
      cout << "channels    - " << properties->channels() << endl;
      cout << "length      - " << minutes << ":" << formatSeconds(seconds) << endl;
    }
  }
  return 0;
}
*/

