/**
 * Author: Ryan Steinwert
 *
 * Implementation for Common Sense database access manager
 */
#define NAME_BUF_SIZE 1024


#include <string.h>

#include "CSDBAccessManager.h"

/**
 * Constructor for access manager type
 */
CSDBAccessManager::CSDBAccessManager()
{

}


/**
 * Deconstructor for access manager
 */
CSDBAccessManager::~CSDBAccessManager()
{
	dbs.clear();
	rms.clear();
}

/**
 * Adds a database with the given name to the current list
 * @param name The name of the new db to add
 * @param rulesFile The rules file to associate with this db
 * @return 0 if successfully added, error code if not
 */
int CSDBAccessManager::addDB(const char* name, const char* rulesFile)
{
	int ret;
	char buf[NAME_BUF_SIZE];

	// check if the db has been added yet
	for(unsigned long i = 0; i < dbs.size(); i++) 
	{
		CSDB* db = dbs[i];
		db->getDBName(buf, NAME_BUF_SIZE);

		if(strcmp(buf, name) == 0) return 0;
	}

	CSDBRuleManager* rm = new CSDBRuleManager();
	ret = rm->loadRules(rulesFile);

	if(ret != 0) return ret;

	dbs.push_back(new CSDB(name));
	rms.push_back(rm);

	return 0;
}