#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Definition of access manager for Common Sense database
 */

#include <vector>

#include "CSDB.h"


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
};