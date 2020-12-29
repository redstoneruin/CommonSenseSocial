/**
 * Test suite for CSDB
 */

#define BUF_SIZE 2048

#include <stdio.h>
#include <string.h>

#include "../CSDB/CSDB.h"

CSDB db;


int additionTests();
int existanceTests();
int deletionTests();
int itemAdditionTests();
int itemExistanceTests();
int itemDeletionTests();
int itemExistanceTests2();
int textItemRetrievalTests();

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

    printf("Item existance tests: ");
    printResult(stdout, itemExistanceTests());

    printf("Item deletion tests: ");
    printResult(stdout, itemDeletionTests());

    printf("Item existance tests 2: ");
    printResult(stdout, itemExistanceTests2());

    printf("Text item retrieval tests:");
    printResult(stdout, textItemRetrievalTests());

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

int existanceTests() 
{
    if(!db.collectionExists("test1/test3")) return -1;
    if(!db.collectionExists("test1/test4/test5")) return -2;
    if(db.collectionExists("test6")) return -3;

    return 0;
}

int deletionTests() 
{
    if(db.deleteCollection("test1/test4/test5") != 0) return -1;
    if(db.deleteCollection("test1/test4") != 0) return -2;
    if(db.deleteCollection("test2") != 0) return -3;
    if(db.deleteCollection("test7") == 0) return -4;
    return 0;
}

int itemAdditionTests() 
{
    int ret;
    if((ret = db.replaceItem("test1/test5/item1", "A basic text item")) != 0) return ret;
    if((ret = db.replaceItem("test1/item1", "A basic text item")) != 0) return ret;
    if((ret = db.replaceItem("test1/item2", "A basic text item")) != 0) return ret;
    if((ret = db.replaceItem("test3/item2", "A basic text item")) != 0) return ret;
    return 0;
}


int itemExistanceTests()
{
    if(!db.itemExists("test1/test5/item1")) return -1;
    if(!db.itemExists("test1/item1")) return -2;
    if(!db.itemExists("test1/item2")) return -3;
    if(!db.itemExists("test3/item2")) return -4;

    if(db.itemExists("test3/item10")) return -5;

    return 0;
}

int itemDeletionTests()
{
    int ret;
    if((ret = db.deleteItem("test1/test5/item1")) != 0) return ret;
    if((ret = db.deleteItem("test1/item1")) != 0) return ret;

    return 0;
}

int itemExistanceTests2()
{
    if(db.itemExists("test1/test5/item1")) return -1;
    if(db.itemExists("test1/item1")) return -2;

    return 0;
}

int textItemRetrievalTests()
{
    int ret;
    DTYPE type;
    char buf[BUF_SIZE];

    if((ret = db.getItemData("test1/item2", buf, &type, BUF_SIZE)) < 0) return ret;
    buf[ret] = 0;
    if(strcmp("A basic text item", buf) != 0) return -10;
    
    if((ret = db.getItemData("test3/item2", buf, &type, BUF_SIZE)) < 0) return ret;
    buf[ret] = 0;
    if(strcmp("A basic text item", buf) != 0) return -11;

    return 0;
}

void printResult(FILE* file, int result) 
{
    if(result == 0) {
        fprintf(file, "success\n");
        return;
    }

    fprintf(file, "FAILURE: %d\n", result);
}