/**
 * Author: Ryan Steinwert
 * 
 * Tests for account manager
 */

#include <cstdio>
#include <cstring>

#include "../AccountManager.h"


AccountManager am;


int insertTests();
int retrievalTests();

void printResult(FILE* file, int testResult);


int main()
{
	FILE* out = stdout;

	fprintf(out, "-------- Account Manager Tests --------\n");

	fprintf(out, "Account insert tests: ");
	printResult(out, insertTests());

	fprintf(out, "Account retrieval tests: ");
	printResult(out, retrievalTests());

}

int insertTests()
{
	int ret;
	if((ret = am.insertAccount("myuid1", "myusername1", "user1@gmail.com", "abcxyz")) != 0) return ret;
	if((ret = am.insertAccount("myuid2", "myusername2", "user2@gmail.com", "abcxyz")) != 0) return ret;

	return 0;
}

int retrievalTests()
{
	int ret;
	char buf[64];

	if((ret = am.getUsername("myuid1", buf, sizeof(buf))) != 0) return ret;
	if(strcmp(buf, "myusername1") != 0) return -10;

	if((ret = am.getUsername("myuid2", buf, sizeof(buf))) != 0) return ret;
	if(strcmp(buf, "myusername2") != 0) return -20;


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