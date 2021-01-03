#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Definition of access manager for Common Sense database
 */

#include <vector>

#include "../definitions.h"

#include "CSDB.h"
#include "CSDBRuleManager.h"



/**
 * CSDBAccessManager class
 */
class CSDBAccessManager {
public:
	CSDBAccessManager();
	~CSDBAccessManager();

	int addDB(const char* name, const char* rulesFile);

	int addCollection(const char* dbName, const char* path, request_info_s requestInfo);
	int deleteCollection(const char* dbName, const char* path, request_info_s requestInfo);

	int replaceItem(const char* dbName, const char* path, request_info_s requestInfo, const char* text, PERM perm = PERM::PRIVATE);
	int deleteItem(const char* dbName, const char* path, request_info_s requestInfo);

	size_t getItemData(const char* dbName, const char* path, request_info_s requestInfo, void* buf, DTYPE* type, size_t bufSize, size_t offset = 0);

	bool collectionExists(const char* dbName, const char* path, request_info_s requestInfo);
	bool itemExists(const char* dbName, const char* path, request_info_s requestInfo);

private:
	std::vector<CSDB*> dbs;

	std::vector<CSDBRuleManager*> rms;

	bool getDBPair(const char* dbName, CSDB** db, CSDBRuleManager** rm);
};