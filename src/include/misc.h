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

char *make_absolute(char *relpath);
int   parse_branches(const char *arg);
void  resolve_path(const char *path,char *abspath, int total_size);
char *concat_paths(const char *src1, const char *src2);

#endif /*MISC_H_*/
