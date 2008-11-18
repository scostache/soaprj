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

#include <string>

#include "hybfs_data.h"


/* used to indicate if the query contains a real path
 * and/or tags
 */
#define HAS_PATH 	0x00000001
#define HAS_FILE 	0x00000002
#define HAS_TAG		0x00000004


/*
 * Makes an absolute path from a relative one. The returned string
 * must be freed.
 */
std::string * make_absolute(const char *relpath);

/* 
 * This should resolve the file path, if we have multiple directories/branches.
 */
std::string * resolve_path(HybfsData *hybfs_core, const char *path, int *brid);


#endif /*MISC_H_*/
