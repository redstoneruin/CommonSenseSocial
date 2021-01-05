/**
 * Author: Ryan Steinwert
 * 
 * Definition for collection tree class
 */

#include <cstdio>
#include <ctime>

#include <ftw.h>

#include "../definitions.h"

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
    void* data;
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



class CollectionTree {
public:
	CollectionTree();
	~CollectionTree();

	void setup(const char* dirname);

    int addCollection(const char* path);
    int deleteCollection(const char* path);

    // collection retrieval
    collection_s* getCollection(const char* path);

    // item helpers
    item_s* getItem(const char* path);
    item_s* getItemFromCollection(collection_s* collection, const char* name);

    int replaceItem(const char* path, const char* text, const char* owner = nullptr, PERM perm = PERM::PRIVATE);
    int replaceItem(const char* path, const void* data, size_t dataSize, DTYPE type, const char* owner = nullptr, PERM perm = PERM::PRIVATE);

    int deleteItem(const char* path);

    int getOwner(const char* path, void* buf, size_t bufSize);
    int getPerm(const char* path, PERM* permPointer);

    size_t getItemData(const char* path, void* returnBuffer, DTYPE* type, size_t bufSize, size_t offset = 0);


    int loadItem(item_s* item);
    void unloadItem(item_s* item);

    void dumpCollections(FILE* file);

private:
	int _numBaseCollections;
	const char* _dirname;

	collection_s** _collections;

	int loadTree(const char* collsFilename, unsigned int extraFlags = 0);

	void setupCollectionManifest(collection_s* collection);
	void createFormattedCollectionsFile(const char* formattedCollFilename);


    item_s* getNewItemStruct(const char* path, const char* owner, PERM perm);
    int addItemToParent(item_s* item);
    int writeItem(item_s* item);
    int updateManifest(collection_s* collection);

    // recursive helpers
    void dumpCollectionsHelper(FILE* file, collection_s* parent, int depth = 0);
    void formattedCollectionsHelper(FILE* file, collection_s* parent);
    void collectionLoadHelper(FILE* file, collection_s* parent);
    void deleteCollectionHelper(collection_s* toDelete);


    static int ftwHelper(const char* path, const struct stat64* statStruct, int info, struct FTW* ftw);

    collection_s* parseCollectionString(const char* collectionString, collection_s* parent);

    // input validation
    bool validCollectionPath(const char* path);
    bool validItemPath(const char* path);

    bool validPathCharacter(char c);

};

