#pragma once
/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Database class definition
 */

#include <stdio.h>
#include <ftw.h>
#include <time.h>

enum PERM {
    PRIVATE,
    UNLISTED,
    PUBLIC
};

enum DTYPE {
    NONE,
    TEXT,
    IMAGE,
    AUDIO,
    VIDEO,
    STREAM,
    AUDIO_STREAM
};

typedef struct item_t {
    char* name;
    char* owner;
    PERM perm;
    DTYPE type;
    time_t createdTime;
    time_t modifiedTime;
    bool loaded;
    void* collection;
    size_t dataSize;
    char* data;
} item_s;

typedef struct collection_t {
    int numSubColls;
    unsigned long long numItems;
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
    int deleteCollection(const char* path);

    int replaceItem(const char* path, const char* text, const char* owner = nullptr, PERM perm = PERM::PRIVATE);
    int deleteItem(const char* path);

    size_t getItemData(const char* path, void* returnBuffer, DTYPE* type, size_t bufSize, size_t offset = 0);

    bool collectionExists(const char* path);
    bool itemExists(const char* path);

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

    // item helpers
    item_s* getItem(const char* path);
    item_s* getItemFromCollection(collection_s* collection, const char* name);

    item_s* getNewItemStruct(const char* path, const char* owner, PERM perm);
    int addItemToParent(item_s* item);
    int writeItem(item_s* item);
    int updateManifest(collection_s* collection);

    int loadItem(item_s* item);
    void unloadItem(item_s* item);

    // recursive helpers
    void dumpCollectionsHelper(FILE* file, collection_s* parent, int depth = 0);
    void formattedCollectionsHelper(FILE* file, collection_s* parent);
    void collectionLoadHelper(FILE* file, collection_s* parent);
    void deleteCollectionHelper(collection_s* toDelete);

    static int ftwHelper(const char* path, const struct stat64* statStruct, int info, struct FTW* ftw);

    collection_s* parseCollectionString(const char* collectionString, collection_s* parent);
};