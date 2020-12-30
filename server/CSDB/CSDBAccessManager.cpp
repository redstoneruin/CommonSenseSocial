/**
 * Author: Ryan Steinwert
 *
 * Implementation for Common Sense database access manager
 */


/**
 * Constructor for access manager type
 */
CSDBAccessManager::CSDBAccessManager()
{

}


CSDBAccessManager::~CSDBAccessManager()
{
	for(int i = 0; i < dbs.size(); i++) {
		delete dbs[i];
	}
	dbs.clear();
}

/**
 * Adds a database with the given name to the current list
 */
void CSDBAccessManager::addDB(const char* name)
{
	dbs.push_back(new CSDB());
}