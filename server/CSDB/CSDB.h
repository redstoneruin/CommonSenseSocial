#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Database class definition
 */

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
    // vars for parsing collections file
    int _currentCollDepth, _collIndex;
    collection_s* _currentParent;

    const char* _dbDirname;

    collection_s** _collections;

    void setup();
    void loadDB(const char* collsFilename);
    
    void parseCollectionString(char* collectionString);
};