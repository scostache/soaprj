/* 
 misc.h - Miscellaneous stuff related to string parsing, branch adding, etc.
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef MISC_H_
#define MISC_H_

/* used for validate_dir to indicate if the query contains a real path
 * and/or tags
 */
#define HAS_PATH 	0x00000001
#define HAS_TAG 	0x00000002

/* the functions from misc.c*/

char *make_absolute(char *relpath);
int parse_branches(const char *arg);
void resolve_path(const char *path, char *abspath,int *brid, int total_size);
char *concat_paths(const char *src1, const char *src2);

int validate_vdir(const char *path, int *flags);

#endif /*MISC_H_*/
