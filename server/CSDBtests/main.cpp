/**
 * Test suite for CSDB
 */

#include <stdio.h>

#include "../CSDB/CSDB.h"

CSDB db;

int main()
{
    printf("Beginning CSDB tests\n");
    db.dumpCollections(stdout);
    return 0;
}