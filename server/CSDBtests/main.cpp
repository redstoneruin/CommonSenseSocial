/**
 * Test suite for CSDB
 */

#include <stdio.h>

#include "../CSDB/CSDB.h"

CSDB db;


int existanceTests();

void printResult(FILE* file, int result);

int main()
{
    printf("----Dumping collection structure----\n");
    db.dumpCollections(stdout);
    
    printf("\nExistance tests: ");
    printResult(stdout, existanceTests());

    return 0;
}


int existanceTests() {
    if(!db.collectionExists("test1/test3")) return -1;
    if(!db.collectionExists("test1/test4/test5")) return -2;
    if(db.collectionExists("test6")) return -3;

    return 0;
}


void printResult(FILE* file, int result) {
    if(result == 0) {
        fprintf(file, "success\n");
        return;
    }

    fprintf(file, "FAILURE: %d\n", result);
}