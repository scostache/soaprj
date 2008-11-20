/* 
 path_crawler.h - Breaks a path into components
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */


#ifndef PATH_CRAWLER_H_
#define PATH_CRAWLER_H_


#include <string>
#include <list>

using namespace std;

class PathCrawler{
private:
	string path;
	size_t found;
	
	string last;
	string rest;
	
	char sep; /* path separator */
	
	list<string> components;
	
public:
	PathCrawler(const char *_path);
	PathCrawler(const char *_path, char _sep);
	~PathCrawler();
	
	/*
	 * returns the next component from the path, separated by "/"
	 * It begins from the end to the begining.
	 */
	string get_next();
	
	/*
	 * returns 1 if there are still unprocessed elements
	 */
	int has_next();
	
	/*
	 * parse the path and adds to the component list the queries
	 * separated by '/(' and ')/' or '/(' and ')' if is the last element
	 */
	int break_queries();
	
};

#endif /*PATH_CRAWLER_H_*/