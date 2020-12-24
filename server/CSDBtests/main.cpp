/**
 * Test suite for CSDB
 */

#include <stdio.h>

#include "../CSDB/CSDB.h"

CSDB db;


int additionTests();
int existanceTests();

void printResult(FILE* file, int result);

int main()
{
    printf("----Dumping collection structure----\n");
    db.dumpCollections(stdout);


    printf("\nAddition tests: ");
    printResult(stdout, additionTests());
    
    printf("Existance tests: ");
    printResult(stdout, existanceTests());

    return 0;
}

int additionTests() {
    int ret;

    if((ret = db.addCollection("test1")) != 0) return -1;
    if((ret = db.addCollection("test2")) != 0) return -2;
    if((ret = db.addCollection("test1/test3")) != 0) return -3;
    if((ret = db.addCollection("test1/test4")) != 0) return -4;
    if((ret = db.addCollection("test1/test4/test5")) != 0) return -5;
    
    // bad format
    if((ret = db.addCollection("test2/test4/test5")) == 0) return -6;

    
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