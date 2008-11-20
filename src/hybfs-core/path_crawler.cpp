/* 
 path_crawler.cpp - breaks a path into components 
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "hybfs.h"
#include "path_crawler.h"

PathCrawler::PathCrawler(const char *_path)
{
	path = _path;
	rest = path;
	sep = '/';

	found = path.find_last_of(sep);
}

PathCrawler::PathCrawler(const char *_path, const char _sep)
{
	path = _path;
	rest = path;
	sep = _sep;

	found = path.find_last_of(sep);
}

PathCrawler::~PathCrawler()
{
	components.clear();
}

int PathCrawler::has_next()
{
	return (found == string::npos && rest.length() == 0) ? 0 : 1;
}

string PathCrawler::get_next()
{
	if(found == string::npos && rest.length() > 0) {
		last = rest;
		rest.clear();
		
		return last;
	}
	last = rest.substr(found+1);
	rest = rest.substr(0, found);

	found = rest.find_last_of(sep);

	return last;
}

int PathCrawler::break_queries()
{
	size_t firstpos, lastpos;
	string respath;

	firstpos = path.find("/(");
	lastpos = path.find(")/");

	if (firstpos != string::npos && lastpos == string::npos
	                && path.at(path.length()-1) == ')') {
		lastpos = path.length()-1;
	}

	/* path with no queries */
	if (firstpos == string::npos || lastpos == string::npos) {
		return 0;
	}
	/* if it's not the first element in the path, push the  begining
	 * in the list */
	if (firstpos != 0)
		components.push_back(path.substr(0, firstpos));

	/* path with one or more queries */
	respath = path;
	while (firstpos != string::npos && lastpos != string::npos) {
		try {
			components.push_back(respath.substr(firstpos+1, lastpos - firstpos));
			if(lastpos == respath.length()-1) {
				respath = "";
				break;
			}
			respath = respath.substr(lastpos+1);
			firstpos = respath.find("/(");
			lastpos = respath.find(")/");
			if (firstpos != string::npos && lastpos == string::npos
					&& respath.at(respath.size()-1) == ')') {
				lastpos = respath.size()-1;
			}
		}
		catch(std::exception) {
			DBG_PRINT("Nasty exception in %s:%d\n", __func__, __LINE__);
			return components.size();
		}
	}
	/* do we have another component that is not a query? */
	if (respath.length() >0)
		components.push_back(respath);

	return components.size();
}

string PathCrawler::pop_next_query()
{
	string current;
	
	current = components.front();
	components.pop_front();
	
	return current;
}

int PathCrawler::has_next_query()
{
	return  (components.size() == 0) ? 0 : 1;
}
