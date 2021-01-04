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
 * @return 0 If collection successfully added or exists already, error code if not
 */
int CSDBAccessManager::addCollection(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "w";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	// check whether has access permissions
	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;


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

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->deleteCollection(path);
}



/**
 * Replace an item in the database
 * @param dbName The name of the database to add to
 * @param path The path of the item to replace
 * @param requestInfo Info for this request
 * @param text The text to add to the item
 * @param perm The permission status to give this item
 * @return 0 if successful, error code if not
 */
int CSDBAccessManager::replaceItem(const char* dbName, const char* path, request_info_s requestInfo, const char* text, PERM perm)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "w";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->replaceItem(path, text, requestInfo.uid, perm);
}

/**
 * Replace an item using a general buffer and type
 * @param dbName Name of the database to add to
 * @param path The path of the item to replce
 * @param requestInfo Info for this request
 * @param data Data buffer to read from
 * @param dataSize Size of data to write to item buffer in bytes
 * @param type Type of the item
 * @param perm The permission status to give this item
 * @return 0 if successful, error code if not
 */
int CSDBAccessManager::replaceItem(const char* dbName, const char* path, request_info_s requestInfo, const void* data, size_t dataSize, DTYPE type, PERM perm)
{
	CSDB* db;
	CSDBRuleManager* rm;
	requestInfo.perms = "w";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->replaceItem(path, data, dataSize, type, requestInfo.uid, perm);
}

/**
 * Return the data of the item at a given path, should the user have access permissions
 * @param dbName The name of the database
 * @param path The path of the item to access
 * @param requestInfo Info for the database request
 * @param buf Buffer to store item data
 * @param type Pointer to item type to store
 * @param bufSize Maximum size to write to buffer
 * @param offset Offset to open file at
 * @return Number of bytes written if successfully, 0 if no bytes available or unsuccessful
 */
size_t CSDBAccessManager::getItemData(const char* dbName, const char* path, request_info_s requestInfo, void* buf, DTYPE* type, size_t bufSize, size_t offset)
{
	PERM perm;
	char nameBuf[NAME_BUF_SIZE];
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "r";

	if(!getDBPair(dbName, &db, &rm)) return 0;

	if(!rm->hasPerms(path, requestInfo)) return 0;

	if(!db->itemExists(path)) return 0;

	// check the perm status of the item, and whether this is the owner
	if(db->getPerm(path, &perm) != 0) return 0;


	if(perm == PERM::PRIVATE) {
		if(db->getOwner(path, nameBuf, NAME_BUF_SIZE) != 0) return 0;
	
		if(strcmp(requestInfo.uid, nameBuf) != 0) return 0;
	}

	return db->getItemData(path, buf, type, bufSize, offset);

}


/**
 * Delete item in database at a given path
 * @param dbName The name of the database to delete from
 * @param path The path of the item
 * @param requestInfo Info about this request
 * @return 0 if successful, error code if not
 */
int CSDBAccessManager::deleteItem(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "w";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->deleteItem(path);
}


/**
 * Returns whether collection exists in the given db
 * @param dbName The name of the database to check
 * @param path The path of the collection
 * @param requestInfo Info about this request
 * @return True if the collection exists, false if does not, or error occured
 */
bool CSDBAccessManager::collectionExists(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "r";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->collectionExists(path);
}


/**
 * Returns whether item exists in the given db
 * @param dbName The name of the database to check
 * @param path The path of the item
 * @param requestInfo Info about this request
 * @return True if the item exists, false if does not, or error occured
 */
bool CSDBAccessManager::itemExists(const char* dbName, const char* path, request_info_s requestInfo)
{
	CSDB* db;
	CSDBRuleManager* rm;

	requestInfo.perms = "r";

	if(!getDBPair(dbName, &db, &rm)) return ERROR::NO_DB;

	if(!rm->hasPerms(path, requestInfo)) return ERROR::NO_PERMS;

	return db->itemExists(path);
}



/**
 * Fills the db and rm pointers for the given db name
 * @param dbName The name of the database to retrieve
 * @param db Pointer to database pointer to fill
 * @param rm Pointer to rule manager pointer to fill
 * @return True if successfully filled pointers, false if not
 */
bool CSDBAccessManager::getDBPair(const char* dbName, CSDB** db, CSDBRuleManager** rm)
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
			return true;
		}
	}

	return false;
}