/**
 * Author: Ryan Steinwert
 *
 * Implementation file for account manager
 */

#include <cstring>
#include <cstdlib>

#include "AccountManager.h"


AccountManager::AccountManager() :
	_root(nullptr)
{

}


AccountManager::~AccountManager()
{
}



int AccountManager::insertAccount(const char* uid, const char* username, const char* email, const char* passhash)
{
	int uidLen, usernameLen, emailLen, passhashLen;
	uidLen = strlen(uid);
	usernameLen = strlen(username);
	emailLen = strlen(email);
	passhashLen = strlen(passhash);

	account_node_s* toInsert = (account_node_s*) malloc (sizeof(account_node_s));

	toInsert->uid = (char*) malloc (uidLen+1);
	toInsert->username = (char*) malloc (usernameLen+1);
	toInsert->email = (char*) malloc (emailLen+1);
	toInsert->passhash = (char*) malloc (passhashLen+1);
	toInsert->left = nullptr;
	toInsert->right = nullptr;

	return insertNode(toInsert);
}



int AccountManager::insertNode(account_node_s* node)
{
	return 0;
}