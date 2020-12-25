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

#define COLLECTIONS_CHILDREN_SIZE 4
#define COLLECTIONS_ITEMS_SIZE 8

#define COLLECTIONS_FILENAME "collections"
#define FORMATTED_COLLECTIONS_FILENAME "formattedCollections"

/**
 * CSDB constructor
 */
CSDB::CSDB() : CSDB("db")
{
}

/**
 * Create db with the given filename
 * @dirname the directory containing the database
 */
CSDB::CSDB(const char* dirname) :
    _numBaseCollections(0)
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

    std::string collFilename(_dbDirname);
    std::string formattedCollFilename(_dbDirname);
    collFilename.push_back('/');
    formattedCollFilename.push_back('/');
    collFilename.append(COLLECTIONS_FILENAME);
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);;

    // attempt to load formatted collections first
    if(loadDB(formattedCollFilename.c_str()) == 0) {
        // formatted collection file found
    } else if(loadDB(collFilename.c_str(), O_CREAT) == 0) {
        // plain collection file found, do formatting
        createFormattedCollectionsFile(formattedCollFilename.c_str());
    } else {
        fprintf(stderr, "Error: Could not find or create collections file\n");
        exit(1);
    }
}


/**
 * Load DB using collections file
 * @param collsFilename Filename for collections
 */
int CSDB::loadDB(const char* collsFilename, unsigned int extraFlags)
{
    int fd;
    FILE* file;
    std::vector<collection_s*> baseCollections;
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + COLLECTIONS_CHILDREN_SIZE + 2];

    fd = open(collsFilename, O_RDWR | extraFlags);

    // should return without error, without message if file not found 
    if(fd == -1) {
        return -1;
    }

    // open file in read only mode
    file = fdopen(fd, "r");
    if(file == nullptr) {
        fprintf(stderr, "Could not open collections file stream\n");
        return -1;
    }

    // read through file parsing by whitespace
    while(fscanf(file, "%s", collNameBuf) == 1) {
        // parse the collection using helper function
        collection_s* parent = parseCollectionString(collNameBuf, nullptr);
        
        setupCollectionManifest(parent);
        
        collectionLoadHelper(file, parent);
        baseCollections.push_back(parent);
    }

    close(fd);


    // write the collections
    _numBaseCollections = baseCollections.size();
    _collections = (collection_s**) malloc (sizeof(collection_s*) * _numBaseCollections);
    memcpy(_collections, baseCollections.data(), _numBaseCollections * sizeof(collection_s*));


    return 0;
}

/**
 * Add a collection with the given path to the collection structure
 * @param path Path for new collection
 * @return 0 if successful, error code if not
 */
int CSDB::addCollection(const char* path)
{

    size_t lastSepIndex;
    std::string pathstring(path);
    std::string formattedCollFilename(_dbDirname);

    // check if collection already exists
    if(collectionExists(path)) return 0;

    formattedCollFilename.push_back('/');
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);

    lastSepIndex = pathstring.find_last_of('/');


    if(lastSepIndex == std::string::npos) {
        // add new base collection
        collection_s* newColl = parseCollectionString(pathstring.append(":0").c_str(), nullptr);

        collection_s** newCollections = (collection_s**) malloc (sizeof(collection_s*) * _numBaseCollections + 1);

        // copy over base collections
        for(int i = 0; i < _numBaseCollections; i++)
        {
            newCollections[i] = _collections[i];
        }

        newCollections[_numBaseCollections] = newColl;

        if(_collections != nullptr) free(_collections);
        _collections = newCollections;
        _numBaseCollections++;

        createFormattedCollectionsFile(formattedCollFilename.c_str());

        return 0;

    }
    
    std::string parentPath = pathstring.substr(0, lastSepIndex);
    std::string name = pathstring.substr(lastSepIndex + 1);
    std::string collstring = pathstring.substr(lastSepIndex + 1);
    collstring.append(":0");


    collection_s* parent = getCollection(parentPath.c_str());

    // ensure parent exists
    if(parent == nullptr) return -2;

    // look for child already existing
    for(int i = 0; i < parent->numSubColls; i++) {
        collection_s* child = parent->subCollections[i];
        // if name found, can return without failure
        if(strcmp(child->name, name.c_str()) == 0) return 0;
    }


    collection_s* child = parseCollectionString(collstring.c_str(), parent);

    // create new subcollection array and copy over old one
    collection_s** subCollections = (collection_s**) malloc (sizeof(collection_s*) * (parent->numSubColls + 1));
    memcpy(subCollections, parent->subCollections, sizeof(collection_s*) * parent->numSubColls);
    
    // add new child
    subCollections[parent->numSubColls] = child;

    // free the old subcollection list and replace
    free(parent->subCollections);
    parent->subCollections = subCollections;
    parent->numSubColls++;

    createFormattedCollectionsFile(formattedCollFilename.c_str());

    return 0;
}


/**
 * Delete the collection at the given path
 * @param path Path of collections to delete
 * @return 0 if deleted successfully, error code if not
 */
int CSDB::deleteCollection(const char* path)
{
    collection_s* toDelete = getCollection(path);

    if(toDelete == nullptr) return -1;

    // delete from parent list
    if(toDelete->parent == nullptr) {
        // find position in base collections
        for(int i = 0; i < _numBaseCollections; i++) 
        {
            if(_collections[i] == toDelete) {
                // remove collection from list
                collection_s** collections = (collection_s**) malloc (sizeof(collection_s*) * (_numBaseCollections -1));
                // copy still valid colls
                for(int j = 0; j < _numBaseCollections; j++) 
                {
                    if(j == i) continue;

                    if(j > i) {
                        collections[j-1] = _collections[j];
                    } else {
                        collections[j] = _collections[j];
                    }
                }

                //memcpy(collections, _collections, i);
                //memcpy(collections+i, _collections+i+1,_numBaseCollections-i-1);
                free(_collections);
                _collections = collections;
                _numBaseCollections--;
                break;
            }
        }
    } else {
        // remove from parent's child collections
        collection_s* parent = toDelete->parent;

        for(int i = 0; i < parent->numSubColls; i++)
        {
            if(parent->subCollections[i] == toDelete) {
                collection_s** subCollections = (collection_s**) malloc (sizeof(collection_s*) * (parent->numSubColls-1));
                
                for(int j = 0; j < parent->numSubColls; j++)
                {
                    if(j==i) continue;
                    if(j > i) {
                        subCollections[j-1] = parent->subCollections[j]; 
                    } else {
                        subCollections[j] = parent->subCollections[j];
                    }
                }
                //memcpy(subCollections, parent->subCollections, i);
                //memcpy(subCollections+i, parent->subCollections+i+1,parent->numSubColls-i-1);
                free(parent->subCollections);
                parent->subCollections = subCollections;
                parent->numSubColls--;
                break;
            }
        }
    }

    deleteCollectionHelper(toDelete);

    std::string formattedCollFilename(_dbDirname);
    formattedCollFilename.push_back('/');
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);

    // rewrite collections file
    createFormattedCollectionsFile(formattedCollFilename.c_str());

    return 0;
}

/**
 * Recursive helper for deleting collection
 * @param toDelete Collection to delete
 */
void CSDB::deleteCollectionHelper(collection_s* toDelete)
{
    // recursively delete children   
    for(int i = 0; i < toDelete->numSubColls; i++) 
    {
        deleteCollectionHelper(toDelete->subCollections[i]);
    }

    // delete items
    for(int i = 0; i < toDelete->numItems; i++)
    {
        // TODO: free item info stored in struct
        free(toDelete->items[i]);
    }

    if(toDelete->items != nullptr) free(toDelete->items);

    free(toDelete);
}

/**
 * Create formatted collections file
 * @param formattedCollFilename Filename for formatted collection file
 */
void CSDB::createFormattedCollectionsFile(const char* formattedCollFilename)
{
    FILE* file;
    if((file = fopen(formattedCollFilename, "w+")) == nullptr) {
        fprintf(stderr, "Could not create formatted collection file\n");
        return;
    }

    for(int i = 0; i < _numBaseCollections; i++) {
        formattedCollectionsHelper(file, _collections[i]);
    }

    fclose(file);
}

/**
 * Add an item with the given path to the collection
 * @param path Path of the new item
 * @param text Test to store in this item
 * @return 0 if successful, error code if not
 */
int CSDB::addItem(const char* path, const char* text)
{

    item_s* item = getNewItemStruct(path);

    if(item == nullptr) {
        return -1;
    }

    return 0;

}


/**
 * Get and fill a new item struct with the given path
 * @param path The path of the item
 * @return 0 if successful, error code if not
 */
item_s* CSDB::getNewItemStruct(const char* path)
{
    std::string pathstring(path);
    size_t lastSep = pathstring.find_last_of('/');
    if(lastSep == std::string::npos) {
        return nullptr;
    }

    collection_s* collection = getCollection(pathstring.substr(0, lastSep).c_str());

    if(collection == nullptr) return nullptr;

    // create the new item struct
    
}


/**
 * Write an item to the file structure
 * @param item The item to write
 * @return 0 if successful, error code if not
 */
int CSDB::writeItem(item_s* item) {
    return 0;
}


/**
 * Recursive helper for writing collections to formatted file
 * @param file File stream to write to, must b open
 * @param parent Parent collection
 */
void CSDB::formattedCollectionsHelper(FILE* file, collection_s* parent)
{
    // print parent
    fprintf(file, "%s:%d ", parent->name, parent->numSubColls);

    for(int i = 0; i < parent->numSubColls; i++) {
        formattedCollectionsHelper(file, parent->subCollections[i]);
    }
}

/**
 * Recursive helper function for filling collections structs
 * @param file File stream to read collections from
 * @param parent Current parent collection
 */
void CSDB::collectionLoadHelper(FILE* file, collection_s* parent)
{
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + 1];

    setupCollectionManifest(parent);

    for(int i = 0; i < parent->numSubColls; i++) {
        if(fscanf(file, "%s", collNameBuf) != 1) {
            printf("Error loading collections, expected extra subcollection\n");
            exit(1);
        }

        collection_s* child = parseCollectionString(collNameBuf, parent);

        // recursive call
        collectionLoadHelper(file, child);

        parent->subCollections[i] = child;
    
    }
}


/**
 * Parse a collection string
 * @param collectionString C string containing name and number of subcollections to parse
 * @param parent Parent collection, can be null
 * @return New collections struct in heap mem containing info from string
 */
collection_s* CSDB::parseCollectionString(const char* collectionString, collection_s* parent)
{
    // find ':' to parse the number of subcollections
    int colPos, nameLen;
    collection_s* newColl = (collection_s*) malloc (sizeof(collection_s));
    newColl->parent = parent;
    newColl->numItems = 0;
    newColl->path = nullptr;
    newColl->subCollections = nullptr;
    newColl->items = nullptr;
    
    nameLen = strlen(collectionString);
    for(int i = 0; i < nameLen; i++) {
        if(collectionString[i] == ':') {
            colPos = i;
        }
    }

    const char* numSubCollsString = collectionString + colPos + 1;

    // fill collection struct
    newColl->numSubColls = atoi(numSubCollsString);
    newColl->name = (char*) malloc (sizeof(char) * (colPos + 1));
    strncpy(newColl->name, collectionString, colPos);
    newColl->name[colPos] = 0;

    // set path based on parent
    std::string pathstring;

    if(parent == nullptr) {
        pathstring.append(_dbDirname);
        pathstring.push_back('/');
        pathstring.append(collectionString);
    } else {
        // create path string
        pathstring.append(parent->path);
        pathstring.push_back('/');
        pathstring.append(collectionString);
    }

    // set path string
    int len = pathstring.length(); 
    newColl->path = (char*) malloc (sizeof(char) * (len + 1));
    strncpy(newColl->path, pathstring.c_str(), len);

    // ensure null termination
    newColl->path[len] = 0;

    // allocate space for subcolls if exist
    if(newColl->numSubColls > 0) {
        newColl->subCollections = (collection_s**) malloc (sizeof(collection_s*) * newColl->numSubColls);
    }

    return newColl;


}

/**
 * Set up directory structure and ensure manifest file created for collection
 * @param coll Collection to set up
 */
void CSDB::setupCollectionManifest(collection_s* collection)
{
    int fd, colonIndex;
    FILE* file;
    char buf[MAX_COLLECTION_NAME_SIZE + COLLECTIONS_ITEMS_SIZE + 2];
    std::string manifestName(collection->path);
    manifestName.push_back('/');
    manifestName.append("Manifest");

    colonIndex = 0;

    // create dir
    mkdir(collection->path, S_IRWXU);

    // attempt to open manifest file
    fd = open(manifestName.c_str(), O_RDWR | O_CREAT);

    if(fd < 0) {
        fprintf(stderr, "Error: Could not open manifest file for collection: %s\n", collection->name);
        return;
    }

    file = fdopen(fd, "r+");    

    if(file == nullptr) {
        fprintf(stderr, "Error: Could not open file descriptor for collection: %s\n", collection->name);
        return;
    }

    if(fscanf(file, "%s", buf) < 1) {
        // write size 0 to manifest
        fprintf(file, "size:0\n");
        collection->numItems = 0;
        return;

    }

    // manifest exists, size in buffer
    for(int i = 0; i < MAX_COLLECTION_NAME_SIZE + 1; i++) {
        if(buf[i] == ':') {
            colonIndex = i;
            break;
        }
    }

    if(colonIndex == 0) {
        fprintf(stderr, "Error: Could not find collection size for collection: %s\n", collection->name);
    }

    // set num items to whats in the manifest
    collection->numItems = atoll(buf + colonIndex + 1);

}


/**
 * Return whether this collection exists or not
 * @param path Path string for the collection
 * @return True if the collection exists, false if does not
 */
bool CSDB::collectionExists(const char* path)
{
    return getCollection(path) != nullptr;
}


/**
 * Find the collection with the given path, returns null if does not exist
 * @param path Path string for the collection
 * @return The collection for the given path, null if DNE
 */
collection_s* CSDB::getCollection(const char* path)
{
    int pathDepth;
    std::string pathstring(path);
    size_t nextSep, lastSep;
    std::vector<char*> collectionNames;
    collection_s *nextParent, *lastParent, *ret;

    ret = nullptr;
    
    // first break path into pieces
    nextSep = lastSep = -1;
    while((nextSep = pathstring.find('/', lastSep + 1)) != std::string::npos) 
    {
        int nameLen = nextSep - lastSep - 1;
        char* name = (char*) malloc (sizeof(char) * (nameLen + 1));

        memcpy(name, path + lastSep + 1, nameLen);
        name[nameLen] = 0;

        collectionNames.push_back(name);
        
        lastSep = nextSep;
    }

    // get the last 
    int nameLen = strlen(path + lastSep + 1);
    char* name = (char*) malloc (sizeof(char) * (nameLen + 1));
    memcpy(name, path + lastSep + 1, nameLen);
    name[nameLen] = 0;
    collectionNames.push_back(name);

    pathDepth = collectionNames.size();

    // attempt to find the collection
    for(int i = 0; i < _numBaseCollections; i++) 
    {
        nextParent = _collections[i];

        // match base collection name
        if(strcmp(nextParent->name, collectionNames[0]) != 0) {
            continue;
        } else if(pathDepth == 1) {
            ret = nextParent;
            break;
        }

        for(int j = 1; j < pathDepth; j++) 
        {
            lastParent = nextParent;
            // check each child, update
            for(int k = 0; k < nextParent->numSubColls; k++)
            {
                collection_s* child = lastParent->subCollections[k];
                if(strcmp(child->name, collectionNames[j]) == 0) {
                    // found next collection
                    nextParent = child;
                    break;
                }
            }

            if(lastParent == nextParent) {
                // not found under this parent
                break;
            } else if(j == pathDepth-1) {
                // matching child found at the max depth, must be result
                ret = nextParent;
                break;
            }
        }
        if(ret != nullptr) break;
    }

    // cleanup
    for(int i = 0; i < pathDepth; i++) {
        free(collectionNames[i]);
    }

    return ret;

}



/**
 * Dumps the collection structure to the given file
 * @param file File to dump collection list
 */
void CSDB::dumpCollections(FILE* file)
{
    for(int i = 0; i < _numBaseCollections; i++) {
        dumpCollectionsHelper(file, _collections[i]);
    }
}

/**
 * Helper function for dumping collections to file stream recursively
 * @param file File stream to dump to
 * @param parent Parent collection to print first
 * @param depth Current recursion depth, defaults to 0
 */
void CSDB::dumpCollectionsHelper(FILE* file, collection_s* parent, int depth)
{
    for(int i = 0; i < depth; i++) {
        fprintf(file, "\t");
    }

    fprintf(file, "%s: %s: %d children\n", parent->name, parent->path, parent->numSubColls);

    for(int i = 0; i < parent->numSubColls; i++) {
        dumpCollectionsHelper(file, parent->subCollections[i], depth+1);
    }
}