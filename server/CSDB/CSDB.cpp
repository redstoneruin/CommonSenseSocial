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
 * Create formatted collections file
 * @param formattedCollFilename Filename for formatted collection file
 */
void CSDB::createFormattedCollectionsFile(const char* formattedCollFilename)
{
    FILE* file;
    printf("Creating formatted collections file\n");
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
 * 
 * @return New collections struct in heap mem containing info from string
 */
collection_s* CSDB::parseCollectionString(char* collectionString, collection_s* parent)
{
    // find ':' to parse the number of subcollections
    int colPos, nameLen;
    collection_s* newColl = (collection_s*) malloc (sizeof(collection_s));
    newColl->parent = parent;
    newColl->path = nullptr;
    newColl->subCollections = nullptr;
    newColl->items = nullptr;
    
    nameLen = strlen(collectionString);
    for(int i = 0; i < nameLen; i++) {
        if(collectionString[i] == ':') {
            colPos = i;
            collectionString[i] = (char)0;
        }
    }

    char* numSubCollsString = collectionString + colPos + 1;

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