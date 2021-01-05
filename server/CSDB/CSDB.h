#pragma once
/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Database class definition
 */

#include "../definitions.h"

#include "CollectionTree.h"

#include <stdio.h>
#include <ftw.h>
#include <time.h>

/**
 * Main database type, provides top level DB control
 */
class CSDB {
public:
    CSDB();
    CSDB(const char* dirname);
    ~CSDB();

    void getDBName(void* buf, size_t bufSize);

    int addCollection(const char* path);
    int deleteCollection(const char* path);

    int replaceItem(const char* path, const char* text, const char* owner = nullptr, PERM perm = PERM::PRIVATE);
    int replaceItem(const char* path, const void* data, size_t dataSize, DTYPE type, const char* owner = nullptr, PERM perm = PERM::PRIVATE);

    int deleteItem(const char* path);

    int getOwner(const char* path, void* buf, size_t bufSize);
    int getPerm(const char* path, PERM* permPointer);

    size_t getItemData(const char* path, void* returnBuffer, DTYPE* type, size_t bufSize, size_t offset = 0);

    bool collectionExists(const char* path);
    bool itemExists(const char* path);

    void dumpCollections(FILE* file);

private:
    const char* _dbDirname;

    CollectionTree _collectionTree;


    void setup();


};