#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "hybfs.h"

HybfsData::HybfsData(char *_mountp)
{
	mountp = _mountp;
	doexit = 0;
	retval = 0;
}

int HybfsData::add_branch(const char * branch)
{
	struct stat buf;
	string *dbpath;
	DbBackend *db;

	DBG_PRINT("adding branch #%s# \n", branch);

	if (lstat(branch, &buf) == -1) {
		PRINT_ERROR("hybfs: Warning! This branch is not valid!");
		return 1;
	}
	if (!S_ISDIR(buf.st_mode)) {
		PRINT_ERROR("hybfs: Warning! This branch is not a directory!");
		return 1;
	}

	branches.push_back(string(branch));
	
	dbpath = new string(branch);
	
	*dbpath += MAINDB;
	db = new DbBackend(dbpath->c_str());
	
	delete db;
	
	databases.push_back(db);

	return 0;
}

int HybfsData::delete_branch(const char * branch)
{
	int i, ret;

	ret = 0;
	for (i=0; i< (int) branches.size(); i++) {
		if (strncmp(branches[i].c_str(), branch, branches[i].size() )
		                == 0) {
			/* delete the branch path */
			branches.erase(branches.begin()+i, branches.begin()+1+i);
			/* delete the associated database handle*/
			databases[i]->db_close_storage();
			databases.erase(databases.begin()+i, databases.begin()+1+i);
			
			return 0;
		}
	}
	PRINT_ERROR("Could not find branch %s \n", branch);

	return 1;
}

/*
 *  Extracts the branches from the options string. 
 */
int HybfsData::parse_branches(const char *arg)
{
	char *buf, *branch;
	char **ptr;

	if (branches.size() != 0)
		return 0;

	ABORT((arg[0] == '\0'), "HybFS: No branches specified! \n");

	/* parse the options. This follows the model: "dir1:dir2:dir3" */
	buf = strdup(arg);
	ptr = (char **)&buf;
	while ((branch = strsep(ptr, ROOT_SEP)) != NULL) {
		if (strlen(branch) == 0)
			continue;
		add_branch(branch);
	}

	free(buf);

	return branches.size();
}

const char * HybfsData::get_branch_path(int brid)
{
	if (brid < 0 || brid > (int) branches.size())
		return NULL;

	return branches[brid].c_str();
}

int HybfsData::start_db_storage()
{
	int ret;
	
	for(int i=0; i< (int) databases.size(); i++) {
		ret = databases[i]->db_init_storage();
		if(ret)
			return ret;
	}
	
	return 0;
}

HybfsData::~HybfsData()
{
	int i;
	
	branches.clear();
	
	for(i=0; i< (int) databases.size(); i++)
		databases[i]->db_close_storage();
	
	databases.clear();
}
