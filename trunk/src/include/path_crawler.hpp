/* 
 path_crawler.hpp - Breaks a path into components
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */


#ifndef PATH_CRAWLER_HPP_
#define PATH_CRAWLER_HPP_


#include <string>
#include <list>

using namespace std;

class PathCrawler{

private:
	
	int nqueries;
	
	string path;
	size_t found;
	
	string last;
	string rest;
	
	char sep; /* path separator */
	/**
	 * the path that follows a query
	 */
	string rel_path;
	/**
	 * the path that is the first component of a query
	 */
	string first_path;
	
	list<string> components;
	
public:
	
	PathCrawler(const char *_path);
	PathCrawler(const char *_path, char _sep);
	~PathCrawler();
	
	/**
	 * returns the relative path to a query, or a combination of queries
	 */
	string get_rel_path() { return rel_path; }
	
	/**
	 * returns the path component from a query, only if it's specified 
	 * as the first element
	 */
	string get_first_path() { return first_path; }
	
	/**
	 * returns 1 if the path component starts with REAL_DIR
	 */
	int is_real();
	
	/**
	 * returns the next component from the path, separated by "/"
	 * It begins from the end to the begining.
	 */
	string get_next();
	
	/**
	 * returns 1 if there are still unprocessed elements
	 */
	int has_next();
	
	/**
	 * parse the path and adds to the component list the queries
	 * separated by '/(' and ')/' or '/(' and ')' if is the last element
	 */
	int break_queries();
	
	/* Returns the number of queries from this path */
	int get_nqueries() { return nqueries; }
	
	/**  
	 * gets the next query from the list of components 
	 */
	string pop_next_query();
	
	/**
	 * returns 1 if there are still elements in the list
	 */
	int has_next_query();
};

#endif /*PATH_CRAWLER_HPP_*/
