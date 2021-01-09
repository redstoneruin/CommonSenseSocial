/**
 * Author: Ryan Steinwert
 * 
 * Tests for account manager
 */

#include <cstdio>

#include "../AccountManager.h"


AccountManager am;


int insertTests();

void printResult(FILE* file, int testResult);


int main()
{
	FILE* out = stdout;

	fprintf(out, "-------- Account Manager Tests --------\n");

	fprintf(out, "Account insert tests: ");
	printResult(out, insertTests());
}

int insertTests()
{
	if(am.insertAccount("myuid1", "myusername1", "user1@gmail.com", "abcxyz") != 0) return -1;
	if(am.insertAccount("myuid2", "myusername2", "user2@gmail.com", "abcxyz") != 0) return -2;

	return 0;
}

/**
 * Print success or FAILED based on given result of test
 */
void printResult(FILE* file, int testResult) 
{
    if(testResult == 0) {
        fprintf(file, "success\n");
    } else {
        fprintf(file, "FAILED: %d\n", testResult);
    }
}