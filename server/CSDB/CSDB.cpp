/**
 * Author: Ryan Steinwert
 * 
 * CSDB class implementation file
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

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
CSDB::CSDB(const char* dirname)
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

    // create collection file if dne
    int collfd = open(collFilename.c_str(), O_RDWR | O_CREAT);
    if(collfd == -1) {
        fprintf(stderr, "Could not open collections file\n");
    }

    close(collfd);
}