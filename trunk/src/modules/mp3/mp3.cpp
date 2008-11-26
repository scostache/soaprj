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

char * Mp3File::mp3_get_artist()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return strdup((tag->artist()).to8Bit().c_str());
	return NULL;
}

char * Mp3File::mp3_get_album()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return strdup((tag->album()).to8Bit().c_str());
	return NULL;
}

char * Mp3File::mp3_get_title()
{
	if (tag != NULL)
		if (tag->artist().isAscii())
			return strdup((tag->title()).to8Bit().c_str());
	return NULL;
}

uint Mp3File::mp3_get_year()
{
	if (tag != NULL)
		return tag->year();
	return NULL;
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

