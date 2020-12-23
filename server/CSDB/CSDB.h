#pragma once
/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Database class definition
 */

#include <stdio.h>

enum PERM {
    PRIVATE,
    UNLISTED,
    PUBLIC
};

enum DTYPE {
    TEXT,
    IMAGE,
    AUDIO,
    VIDEO,
    STREAM,
    AUDIO_STREAM
};

union ItemData {
    char* text;
    char* location;
};

typedef struct item_t {
    char* name;
    char* owner;
    PERM perms;
    DTYPE type;
    bool loaded;
    union ItemData data;
} item_s;

typedef struct collection_t {
    int numSubColls;
    char* name;
    char* path;
    item_s** items;
    collection_t** subCollections;
    collection_t* parent;
} collection_s;


/**
 * Main database type, provides top level DB control
 */
class CSDB {
public:
    CSDB();
    CSDB(const char* dirname);
    ~CSDB();


private:
    int _numBaseCollections;

    const char* _dbDirname;

    collection_s** _collections;

    void setup();

    void loadDB(const char* collsFilename);
    void collectionLoadHelper(FILE* file, collection_s* parent);
    
    collection_s* parseCollectionString(char* collectionString, collection_s* parent);
};