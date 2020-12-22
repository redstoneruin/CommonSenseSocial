/**
 * Author: Ryan Steinwert
 * 
 * CSDB class implementation file
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>

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
    _currentCollDepth(0),
    _collIndex(0),
    _currentParent(nullptr)
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
 * @collsFilename Filename for collections
 */
void CSDB::loadDB(const char* collsFilename)
{
    int fd;
    FILE* file;
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
        parseCollectionString(collNameBuf);
    }

    close(fd);
}


/**
 * Parse a collection string
 * @collectionString C string containing name and number of subcollections to parse
 */
void CSDB::parseCollectionString(char* collectionString)
{
    // find ':' to parse the number of subcollections
    int colPos, nameLen;
    collection_s* newColl = (collection_s*) malloc (sizeof(collection_s));
    newColl->parent = nullptr;
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
    if(_currentParent == nullptr) {
        newColl->parent = nullptr;
        newColl->path = (char*) malloc (sizeof(char) * (colPos + 1));
        strncpy(newColl->path, collectionString, colPos + 1);
    } else {
        newColl->parent = _currentParent;
        
        // create path string
        std::string pathstring(_currentParent->path);
        pathstring.append("/");
        pathstring.append(collectionString);

        newColl->path = (char*) malloc (sizeof(char) * (pathstring.length() + 1));
        strncpy(newColl->path, pathstring.c_str(), pathstring.length() + 1);

    }

    printf("---Added collection---\n");
    printf("Name: %s\n", newColl->name);
    printf("Path: %s\n", newColl->path);
    printf("Num subcollections: %d\n", newColl->numSubColls);
    printf("Parent: %#x\n\n", (unsigned long long)newColl->parent);




}