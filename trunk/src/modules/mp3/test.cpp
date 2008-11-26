/*
 * test.cpp
 *
 *  Created on: Nov 25, 2008
 *      Author: dan
 */

#include <cstdio>
#include "mp3.h"

int main(int argc, char** argv)
{



	Mp3File mp3_file("Star Guitar.mp3");
	printf ("%s | %s | %s | %u\n", mp3_file.mp3_get_artist(), mp3_file.mp3_get_title(), mp3_file.mp3_get_album(), mp3_file.mp3_get_year());
	mp3_file.mp3_load_file("Star Guitar.mp3");
	printf ("%s | %s | %s | %u\n", mp3_file.mp3_get_artist(), mp3_file.mp3_get_title(), mp3_file.mp3_get_album(), mp3_file.mp3_get_year());
	//printf ("%s .... artist\n", mp3_get_artist("Star Guitar.mp3"));
	return 0;
}
