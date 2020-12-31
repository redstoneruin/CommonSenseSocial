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

	void addDB(const char* name);
private:
	std::vector<CSDB*> dbs;

	CSDBRuleManager ruleManager;
};