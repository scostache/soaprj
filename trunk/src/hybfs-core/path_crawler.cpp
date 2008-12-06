/* 
 path_crawler.cpp - breaks a path into components and parses the result so that
 		    it can be used in the DB.
 
 Copyright (C) 2008-2009  Stefania Costache

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <sstream>

#include "hybfs.h"
#include "misc.h"
#include "path_crawler.hpp"
#include <cstring>

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

int  PathCrawler::is_real() 
{ 
	int pos = first_path.find(REAL_DIR,1, (strlen(REAL_DIR)-1));
	printf("real dir=%s first path= %s my position = %d \n",REAL_DIR, first_path.c_str(), pos);
	return ( pos == 1) ? 1 : 0; 
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
	/* if the query it's not the first element in the path, save the  begining
	 * ( the real path) in the list */
	if (firstpos != 0)
		first_path = path.substr(0, firstpos);

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
	rel_path = respath;
	
	nqueries = components.size();
	
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

std::string * PathCrawler::db_build_sql_query()
{
	string * result;
	ostringstream sql_query;
	string tag_value;
	string tag;
	string value;
	int i, size;

	size = components.size();
	i = 0;
	boost::char_separator<char> sep(" ", "()+|!", boost::keep_empty_tokens);
	
	for (list<string>::iterator iter = components.begin(); iter
		                != components.end(); iter++) {
	i++;
	Tok t(*iter, sep);
	sql_query << "SELECT ino, path, mode FROM files WHERE ";
	for(Tok::iterator beg=t.begin(); beg!=t.end();++beg) {
		if((*beg).length() == 0)
			continue;
		
		switch((*beg)[0]) {
			case '(':
			case ')':
				sql_query << *beg;
				break;
			case '+': 
				sql_query << " AND ";
				break;
			case '|':
				sql_query << " OR ";
				break;
			case '!':
				sql_query << " NOT ";
				break;
			default:
				 /* split the tag:value in pieces */
				tag_value = *beg;
				 break_tag(&tag_value, &tag, &value);
				 sql_query << "files.tags LIKE '% " << tag_value; 
				 if(value.length() > 0)
					 sql_query << " %'";
				 else
					 sql_query << ":%";
				break;
		}
	}
	
	 if(i != size)
		 sql_query << " INTERSECT ";
	}
	
	sql_query << " ;";
	
	result = new string(sql_query.str());
	
	DBG_PRINT("The result query is %s\n", result->c_str());
	
	return result;
}

