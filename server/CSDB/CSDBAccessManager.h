#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Definition of access manager for Common Sense database
 */

#include <vector>

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
private:
	std::vector<CSDB*> dbs;

	std::vector<CSDBRuleManager*> rms;
};