/* 
 path_crawler.hpp - Breaks a path into components.
 
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

#include <boost/tokenizer.hpp>

#include "hybfsdef.h"

namespace hybfs {

using namespace std;

typedef boost::tokenizer<boost::char_separator<char> > Tok;


/**
 * @class PathCrawler
 * @brief Class for parsing a path and extracting the virtual components
 * (the queries) and the real ones. It also maps all the queries to a SQL
 * query that can be performed on the database.
 */
class PathCrawler{

private:
	/**
	 * The total number of queries from this string.
	 */
	int nqueries;
	/**
	 * The string that must be parsed.
	 */
	string path;
	
	size_t found;
	
	string last;
	string rest;
	
	/**
	 * Path separator 
	 */
	char sep;
	/**
	 * The real path from this string, that is next to a query.
	 */
	string rel_path;
	/**
	 * The real path from this string that is previous to a query.
	 */
	string first_path;
	/**
	 * The list of query components from this string.
	 */
	list<string> components;
	
public:
	
	PathCrawler(const char *_path);
	PathCrawler(const char *_path, char _sep);
	~PathCrawler();
	
	/**
	 * @brief Returns the relative path from a string containing a query,
	 * or a combination of queries
	 */
	string get_rel_path() { return rel_path; }
	
	/**
	 * @brief
	 * Returns the first path component from a string containing a query,
	 * or a combination of queries
	 */
	string get_first_path() { return first_path; }
	
	/**
	 * @brief
	 * Returns 1 if the path component starts with REAL_DIR. This means that
	 * the path contains a real component.
	 */
	int is_real();
	
	/**
	 * @brief
	 * Returns the next component from the path, separated by "/"
	 */
	string get_next();
	
	/**
	 * @brief
	 * Returns 1 if there are still unprocessed elements
	 */
	int has_next();
	
	/**
	 * @brief
	 * Parse the string path and adds to the component list the queries
	 * separated by '/(' and ')/' or '/(' and ')' in the case of the last
	 * element
	 */
	int break_queries();
	
	/**
	 * @brief
	 * Returns the number of queries from this path.
	 */
	int get_nqueries() { return components.size(); }
	
	/** 
	 * @brief 
	 * Returns the next query from the list of components.It also removes it
	 * from the list.
	 */
	string pop_next_query();
	
	/**
	 * @brief
	 * Returns 1 if there are still elements in the list.
	 */
	int has_next_query();
	
	
	/**
	 * @brief
	 * This builds an SQL query from all the queries specified in this path.
	 * It returns the query to be processed.
	 */
	std::string *db_build_sql_query(vector<tag_info_t> *tags);
};

}

#endif /*PATH_CRAWLER_HPP_*/
