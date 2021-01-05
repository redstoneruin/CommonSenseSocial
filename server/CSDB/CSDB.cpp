/**
 * Author: Ryan Steinwert
 * 
 * CSDB class implementation file
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>

#include "../definitions.h"

#include "CSDB.h"



/**
 * CSDB constructor
 */
CSDB::CSDB() : CSDB("db")
{
}

/**
 * Create db with the given filename
 * @param dirname the directory containing the database
 */
CSDB::CSDB(const char* dirname)
{
    _dbDirname = dirname;

    setup();
}

/**
 * CSDB deconstructor
 */
CSDB::~CSDB()
{
}


/**
 * DB setup function, loads structure
 */
void CSDB::setup()
{
    mkdir(_dbDirname, S_IRWXU);

    _collectionTree.setup(_dbDirname);

}


/**
 * Store the DB directory name in the given buffer
 * @param buf Buffer to store name in
 * @param bufSize Maximum size to write to buffer
 */
void CSDB::getDBName(void* buf, size_t bufSize)
{
    strncpy((char*)buf, _dbDirname, bufSize);
}



/**
 * Add a collection with the given path to the collection structure
 * @param path Path for new collection
 * @return 0 if successful, error code if not
 */
int CSDB::addCollection(const char* path)
{
    return _collectionTree.addCollection(path);
}


/**
 * Delete the collection at the given path
 * @param path Path of collections to delete
 * @return 0 if deleted successfully, error code if not
 */
int CSDB::deleteCollection(const char* path)
{
   return _collectionTree.deleteCollection(path); 
}

/**
 * Check if the item at a given path exists
 * @param path Path of the item
 * @return True if exists, false if does not
 */
bool CSDB::itemExists(const char* path)
{
    return _collectionTree.getItem(path) != nullptr;
}


/**
 * Get item data from the item at a given path
 * @param path The path of the item
 * @param returnBuffer Buffer to write item data to
 * @param type Pointer to item type, filled with the type of the returned item data
 * @param bufSize The maximum size to write to the buffer
 * @param offset The offset to open the file with
 * @return The number of bytes written
 */
size_t CSDB::getItemData(const char* path, void* returnBuffer, DTYPE* type, size_t bufSize, size_t offset)
{
    return _collectionTree.getItemData(path, returnBuffer, type, bufSize, offset); 
}





/**
 * Add an item with the given path to the collection
 * @param path Path of the new item
 * @param text Test to store in this item
 * @return 0 if successful, error code if not
 */
int CSDB::replaceItem(const char* path, const char* text, const char* owner, PERM perm)
{
    return _collectionTree.replaceItem(path, text, owner, perm);

}


int CSDB::replaceItem(const char* path, const void* data, size_t dataSize, DTYPE type, const char* owner, PERM perm)
{
    return _collectionTree.replaceItem(path, data, dataSize, type, owner, perm);
}


/**
 * Delete the item at the given path
 * @param path The path of the item
 * @return 0 if successfully deleted, error code if not
 */
int CSDB::deleteItem(const char* path)
{
    return _collectionTree.deleteItem(path);
}


/**
 * Get the owner of the item at a given path
 * @param path The path of the item
 * @param ownerBuf Buffer to store owner string in
 * @param bufSize The maximum size of the buffer to write into
 * @return 0 if successfully wrote owner, error code if error
 */
int CSDB::getOwner(const char* path, void* buf, size_t bufSize)
{
    return _collectionTree.getOwner(path, buf, bufSize); 
}


/**
 * Get the permissions for an item
 * @param path The path of the item
 * @param permPointer Pointer to perm item to store result
 * @return 0 if successful, error code if not
 */
int CSDB::getPerm(const char* path, PERM* permPointer)
{
    return _collectionTree.getPerm(path, permPointer);
}






/**
 * Return whether this collection exists or not
 * @param path Path string for the collection
 * @return True if the collection exists, false if does not
 */
bool CSDB::collectionExists(const char* path)
{
    return _collectionTree.getCollection(path) != nullptr;
}




/**
 * Dumps the collection structure to the given file
 * @param file File to dump collection list
 */
void CSDB::dumpCollections(FILE* file)
{
    _collectionTree.dumpCollections(file); 
}



