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
	for(unsigned long i = 0; i < dbs.size(); i++) {
		delete dbs[i];
	}
	dbs.clear();
}

/**
 * Adds a database with the given name to the current list
 * @param name The name of the new db to add
 */
void CSDBAccessManager::addDB(const char* name)
{
	char buf[NAME_BUF_SIZE];

	// check if the db has been added yet
	for(unsigned long i = 0; i < dbs.size(); i++) 
	{
		CSDB* db = dbs[i];
		db->getDBName(buf, NAME_BUF_SIZE);

		if(strcmp(buf, name) == 0) return;
	}

	dbs.push_back(new CSDB());
}