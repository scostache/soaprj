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

class Mp3File
{
private:
	TagLib::FileRef *f;
	TagLib::Tag *tag;
public:
	/* default constructor */
	Mp3File ();

	/* special constructor */
	Mp3File (char * file);

	/* loads the tag from the file
	 * given as argument
	 * returns 0 for success, -1 for error
	 */
	int mp3_load_file(char * file);

	/* artist information */
	char * mp3_get_artist();

	/* title information */
	char * mp3_get_title();

	/* album information */
	char * mp3_get_album();

	/* album year information */
	uint mp3_get_year();
};
