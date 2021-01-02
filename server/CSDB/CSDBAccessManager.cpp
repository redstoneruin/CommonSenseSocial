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

	if(ret != 0) {
		delete rm;
		return ret;
	}

	dbs.push_back(new CSDB(name));
	rms.push_back(rm);

	return 0;
}


/**
 * Add a collection to the db with the given name
 * @param dbName The name of the database
 * @param path The path of the collection to add
 * @param requestInfo Info for this request
 */
int CSDBAccessManager::addCollection(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "w";

	getDBPair(dbName, &db, &rm);

	if(db == nullptr || rm == nullptr) return -1;

	// check whether has access permissions
	if(!rm->hasPerms(path, requestInfo)) return -2;


	return db->addCollection(path);

}


/**
 * Remove a collection from the db with the given name
 * @param dbName The name of the database
 * @param path The path of the collection to remove
 * @param requestInfo Info for this request
 * @return 0 if successful, error code if not
 */
int CSDBAccessManager::deleteCollection(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "w";

	getDBPair(dbName, &db, &rm);

	if(db == nullptr || rm == nullptr) return -1;

	if(!rm->hasPerms(path, requestInfo)) return -2;

	return db->deleteCollection(path);
}



/**
 * Fills the db and rm pointers for the given db name
 * @param dbName The name of the database to retrieve
 * @param db Pointer to database pointer to fill
 * @param rm Pointer to rule manager pointer to fill
 */
void CSDBAccessManager::getDBPair(const char* dbName, CSDB** db, CSDBRuleManager** rm)
{
	char nameBuf[NAME_BUF_SIZE];

	*db = nullptr;
	*rm = nullptr;

	for(unsigned long i = 0; i < dbs.size(); i++) 
	{
		dbs[i]->getDBName(nameBuf, NAME_BUF_SIZE);
		if(strcmp(nameBuf, dbName) == 0) {
			*db = dbs[i];
			*rm = rms[i];
			break;
		}
	}

}