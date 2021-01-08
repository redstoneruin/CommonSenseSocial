/**
 * Author: Ryan Steinwert
 *
 * Tests for session manager module
 */

#include <cstdio>
#include <cstring>


#include "../SessionManager.h"

SessionManager sm;

int createTests();
int deleteTests();
int uidTests();

void printResult(FILE* file, int testResult);

int main()
{
	FILE* out = stdout;


	fprintf(out, "Session create tests: ");
	printResult(out, createTests());

	fprintf(out, "Session delete tests: ");
	printResult(out, deleteTests());

	fprintf(out, "UID tests: ");
	printResult(out, uidTests());
}

int createTests()
{
	uint32_t id1, id2;
	session_s *sess1, *sess2;

	id1 = sm.createSession();
	id2 = sm.createSession();

	if(id1 == id2) return -1;

	sess1 = sm.getSession(id1);
	sess2 = sm.getSession(id2);

	if(sess1 == sess2) return -2;

	if(sess1 == nullptr) return -3;
	if(sess2 == nullptr) return -4;

	if(sess1->id != id1) return -5;
	if(sess2->id != id2) return -6;

	return 0;

}


int deleteTests()
{
	uint32_t id1, id2;

	id1 = sm.createSession();
	id2 = sm.createSession();

	if(!sm.getSession(id1)) return -1;
	if(!sm.getSession(id2)) return -2;

	if(sm.deleteSession(id1) != 0) return -3;
	if(sm.deleteSession(id2) != 0) return -4;

	if(sm.getSession(id1)) return -5;
	if(sm.getSession(id2)) return -6;

	return 0;
}



int uidTests()
{
	uint32_t id1, id2;
	session_s *sess1, *sess2;

	id1 = sm.createSession();
	id2 = sm.createSession();

	if(sm.replaceUid(id1, "myuid1") != 0) return -3;
	if(sm.replaceUid(id2, "myuid2") != 0) return -4;

	sess1 = sm.getSession(id1);
	sess2 = sm.getSession(id2);

	if(strcmp(sess1->uid, "myuid1") != 0) return -5;
	if(strcmp(sess2->uid, "myuid2") != 0) return -6;

	sm.deleteSession(id1);
	sm.deleteSession(id2);

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