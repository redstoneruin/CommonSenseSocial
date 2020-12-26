/**
 * Test suite for CSDB
 */

#include <stdio.h>

#include "../CSDB/CSDB.h"

CSDB db;


int additionTests();
int existanceTests();
int deletionTests();
int itemAdditionTests();

void printResult(FILE* file, int result);

int main()
{
    printf("----Dumping collection structure----\n");
    db.dumpCollections(stdout);


    printf("\nAddition tests: ");
    printResult(stdout, additionTests());
    
    printf("Existance tests: ");
    printResult(stdout, existanceTests());

    printf("Deletion tests: ");
    printResult(stdout, deletionTests());

    printf("Item addition tests: ");
    printResult(stdout, itemAdditionTests());

    printf("\n---- Dumping collection structure---\n");
    db.dumpCollections(stdout);

    return 0;
}

int additionTests() {
    int ret;

    if((ret = db.addCollection("test1")) != 0) return ret;
    if((ret = db.addCollection("test2")) != 0) return ret;
    if((ret = db.addCollection("test1/test3")) != 0) return ret;
    if((ret = db.addCollection("test1/test4")) != 0) return ret;
    if((ret = db.addCollection("test1/test4/test5")) != 0) return ret;
    if((ret = db.addCollection("test3")) != 0) return ret;
    if((ret = db.addCollection("test1/test5")) != 0) return ret;

    // bad format
    if((ret = db.addCollection("test2/test4/test5")) == 0) return ret;

    
    return 0;
}

int existanceTests() {
    if(!db.collectionExists("test1/test3")) return -1;
    if(!db.collectionExists("test1/test4/test5")) return -2;
    if(db.collectionExists("test6")) return -3;

    return 0;
}

int deletionTests() {
    if(db.deleteCollection("test1/test4/test5") != 0) return -1;
    if(db.deleteCollection("test1/test4") != 0) return -2;
    if(db.deleteCollection("test2") != 0) return -3;
    if(db.deleteCollection("test7") == 0) return -4;
    return 0;
}

int itemAdditionTests() {
    if(db.addItem("test1/test5/item1", "A basic text item") != 0) return -1;
    if(db.addItem("test3/item2", "A second basic text item") != 0) return -2;
    return 0;
}


void printResult(FILE* file, int result) {
    if(result == 0) {
        fprintf(file, "success\n");
        return;
    }

    fprintf(file, "FAILURE: %d\n", result);
}