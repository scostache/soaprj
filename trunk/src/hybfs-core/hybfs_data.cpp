#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "hybfs.h"
#include "misc.h"

HybfsData::HybfsData(char *_mountp)
{
	mountp = _mountp;
	doexit = 0;
	retval = 0;
}

int HybfsData::add_branch(const char * branch)
{
	struct stat buf;
	char *abspath;
	string *dbpath;
	VirtualDirectory *vdir;

	DBG_PRINT("adding branch #%s# \n", branch);
	abspath = make_absolute((char*)branch);
	if(abspath == NULL) {
		return -1;
	}
	
	if (lstat(abspath, &buf) == -1) {
		PRINT_ERROR("hybfs: Warning! This branch is not valid!");
		return -1;
	}
	if (!S_ISDIR(buf.st_mode)) {
		PRINT_ERROR("hybfs: Warning! This branch is not a directory!");
		return -1;
	}

	branches.push_back(string(abspath));
	
	dbpath = new string(abspath);
	
	*dbpath += METADIR;
	/* check if the directory exists, if not - create */
	if (lstat(dbpath->c_str(), &buf) == -1) {
		DBG_PRINT("Error at checking directory! Atempt to create one!\n");
		/* TODO get appropriate permisions for our directory */
		if(mkdir(dbpath->c_str(),0755)) {
			perror("Failed to create directory: ");
			return -1;
		}
	}
	
	*dbpath += MAINDB;
	
	DBG_PRINT("path for the database main file: %s \n", dbpath->c_str());
	
	vdir = new VirtualDirectory(dbpath->c_str());
	
	delete dbpath;
	
	vdirs.push_back(vdir);

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
			/* delete the associated vdir handle*/
			vdirs.erase(vdirs.begin()+i, vdirs.begin()+1+i);
			
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

int HybfsData::get_nlinks()
{
	int i, nlinks;
	int size;
	struct stat st;
	int ret;
	
	memset(&st, 0, sizeof(st));
	
	size = branches.size();
	nlinks = 0;
	
	for(i = 0; i< size; i++) {
		ret = lstat(branches[i].c_str(), &st);
		if(!ret)
			nlinks += st.st_nlink;
	}
	
	return nlinks;
}

int HybfsData::virtual_readdir(const char *query, void *buf, fuse_fill_dir_t filler)
{
	int i, size;
	int ret = 0;
	
	size = vdirs.size();
	/* call a virtual readdir for each branch */
	for(i=0; i<size; i++) {
		ret = vdirs[i]->vdir_readdir(query+1, buf, filler);
		if(ret)
			break;
	}
	
	return ret;
}

int HybfsData::start_db_storage()
{
	int ret;
	
	for(int i=0; i< (int) vdirs.size(); i++) {
		ret = vdirs[i]->init();
		if(ret)
			return ret;
	}
	
	return 0;
}

HybfsData::~HybfsData()
{
	branches.clear();
	try{
	for(int i=0; i< (int) vdirs.size(); i++) {
		delete vdirs[i];
	}
	} catch(exception) {
		PRINT_ERROR("Failed to destroy FS data!\n");
	}
	vdirs.clear();
}
