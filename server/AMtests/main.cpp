/**
 * Author: Ryan Steinwert
 * 
 * Tests for account manager
 */

#include <cstdio>
#include <cstring>

#include "../AccountManager.h"


FILE* out;
AccountManager am;



int insertTests();
int retrievalTests();
int deletionTests();
int additionTests();
int findTests();
int loginTests();

void printResult(FILE* file, int testResult);


int main()
{
	out = stdout;

	fprintf(out, "-------- Account Manager Tests --------\n");

	fprintf(out, "Account insert tests: ");
	printResult(out, insertTests());

	fprintf(out, "Account retrieval tests: ");
	printResult(out, retrievalTests());

	fprintf(out, "Account delete tests: ");
	printResult(out, deletionTests());

	fprintf(out, "Account addition tests: ");
	printResult(out, additionTests());

	fprintf(out, "Account find tests: ");
	printResult(out, findTests());

	fprintf(out, "Login tests: ");
	printResult(out, loginTests());
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


int deletionTests()
{
	int ret;
	if((ret = am.deleteAccount("myuid1")) != 0) return ret;
	if((ret = am.deleteAccount("myuid2")) != 0) return ret;

	if(am.accountExists("myuid1")) return -1;
	if(am.accountExists("myuid2")) return -2;

	return 0;
}

int additionTests()
{
	int ret;
	ret = am.createAccount("myusername", "user1@gmail.com", "password");

	fprintf(out, "Returned with [%d]: ", ret);

	return 0;
}


int findTests()
{
	int ret;
	char buf1[512];
	char buf2[512];
	if((ret = am.getUidFromUsername("myusername", buf1, 512))) return ret;
	if((ret = am.getUidFromEmail("user1@gmail.com", buf2, 512))) return ret;

	if(strcmp(buf1, buf2)) return -10;

	fprintf(out, "%s: ", buf1);

	return 0;
}


int loginTests()
{
	int err;
	account_info_s* accountInfo;

	accountInfo = am.login("myusername", "password", &err);

	if(!accountInfo) return err;

	if(strcmp(accountInfo->username, "myusername") != 0) return -10;
	if(strcmp(accountInfo->email, "user1@gmail.com") != 0) return -20;
	fprintf(out, "%s: ", accountInfo->uid);

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