#ifndef HYBFS_DATA_H_
#define HYBFS_DATA_H_

#include <vector>
#include <string>

#include "hybfsdef.h"
#include "db_backend.h"

using namespace std;

class HybfsData
{
private:
	char *mountp;
	vector <string> branches;
	vector<DbBackend *> databases;

public:
	HybfsData(char *mountp);
	~HybfsData();

	int add_branch(const char * path);
	int delete_branch(const char * path);
	int parse_branches(const char *arg);

	const char * get_branch_path(int brid);
	
	int start_db_storage();

	int doexit;
	int retval;
};

#endif /*HYBFS_DATA_H_*/
