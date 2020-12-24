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
    long long numItems;
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

    int addCollection(const char* path);

    bool collectionExists(const char* path);

    void dumpCollections(FILE* file);

private:
    int _numBaseCollections;

    const char* _dbDirname;

    collection_s** _collections;

    void setup();

    // setup functions
    int loadDB(const char* collsFilename, unsigned int extraFlags = 0);
    void setupCollectionManifest(collection_s* collection);
    void createFormattedCollectionsFile(const char* formattedCollFilename);
    
    // collection retrieval
    collection_s* getCollection(const char* path);

    // recursive helpers
    void dumpCollectionsHelper(FILE* file, collection_s* parent, int depth = 0);
    void formattedCollectionsHelper(FILE* file, collection_s* parent);
    void collectionLoadHelper(FILE* file, collection_s* parent);

    collection_s* parseCollectionString(const char* collectionString, collection_s* parent);
};