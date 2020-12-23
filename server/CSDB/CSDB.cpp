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
    collFilename.append("/collections");


    loadDB(collFilename.c_str());
}


/**
 * Load DB using collections file
 * @param collsFilename Filename for collections
 */
void CSDB::loadDB(const char* collsFilename)
{
    int fd;
    FILE* file;
    std::vector<collection_s*> baseCollections;
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + 1];

    fd = open(collsFilename, O_RDWR | O_CREAT);
    
    if(fd == -1) {
        fprintf(stderr, "Could not open collections file\n");
        return;
    }

    // open file in read only mode
    file = fdopen(fd, "r");
    if(file == nullptr) {
        fprintf(stderr, "Could not open collections file stream\n");
        return;
    }

    // read through file parsing by whitespace
    while(fscanf(file, "%s", collNameBuf) == 1) {
        // parse the collection using helper function
        collection_s* parent = parseCollectionString(collNameBuf, nullptr);
        collectionLoadHelper(file, parent);
        baseCollections.push_back(parent);
    }

    close(fd);


    // write the collections
    _numBaseCollections = baseCollections.size();
    _collections = (collection_s**) malloc (sizeof(collection_s*) * _numBaseCollections);
    memcpy(_collections, baseCollections.data(), _numBaseCollections * sizeof(collection_s*));
}


/**
 * Recursive helper function for filling collections structs
 * @param file File stream to read collections from
 * @param parent Current parent collection
 */
void CSDB::collectionLoadHelper(FILE* file, collection_s* parent)
{
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + 1];

    for(int i = 0; i < parent->numSubColls; i++) {
        if(fscanf(file, "%s", collNameBuf) != 1) {
            printf("Error loading collections, expected extra subcollection\n");
            return;
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
    strncpy(newColl->name, collectionString, colPos + 1);

    // set path based on parent
    if(parent == nullptr) {
        newColl->path = (char*) malloc (sizeof(char) * (colPos + 1));
        strncpy(newColl->path, collectionString, colPos + 1);
    } else {
        // create path string
        std::string pathstring(parent->path);
        pathstring.push_back('/');
        pathstring.append(collectionString);

        newColl->path = (char*) malloc (sizeof(char) * (pathstring.length() + 1));
        strncpy(newColl->path, pathstring.c_str(), pathstring.length() + 1);

    }

    // allocate space for subcolls if exist
    if(newColl->numSubColls > 0) {
        newColl->subCollections = (collection_s**) malloc (sizeof(collection_s*) * newColl->numSubColls);
    }

   // printf("---Added collection---\n");
   // printf("Name: %s\n", newColl->name);
   // printf("Path: %s\n", newColl->path);
   // printf("Num subcollections: %d\n", newColl->numSubColls);
   // printf("Parent: %#x\n\n", (unsigned long long)newColl->parent);

    return newColl;


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

void CSDB::dumpCollectionsHelper(FILE* file, collection_s* parent, int depth)
{
    for(int i = 0; i < depth; i++) {
        fprintf(file, "\t");
    }

    fprintf(file, "%s: %d children\n", parent->name, parent->numSubColls);

    for(int i = 0; i < parent->numSubColls; i++) {
        dumpCollectionsHelper(file, parent->subCollections[i], depth+1);
    }
}