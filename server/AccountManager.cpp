/**
 * Author: Ryan Steinwert
 *
 * Implementation file for account manager
 */

#define DEFAULT_TABLE_SIZE 64

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "AccountManager.h"

#include "definitions.h"


AccountManager::AccountManager(uint16_t tableSize) :
	_tableSize(tableSize),
	_table(nullptr)
{
	_table = (account_node_s**) malloc (sizeof(account_node_s*) * tableSize);
	for(int i = 0; i < tableSize; i++)
	{
		_table[i] = nullptr;
	}
}


AccountManager::AccountManager() : AccountManager(DEFAULT_TABLE_SIZE)
{
}


AccountManager::~AccountManager()
{
}


/**
 * Insert account information into the hash table
 * @param uid User id
 * @param username Username to insert
 * @param email Email address for user
 * @param passhash Hash of the user's password
 * @return 0 if successfully inserted, error code if not
 */
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
	toInsert->next = nullptr;

	strncpy(toInsert->uid, uid, uidLen+1);
	strncpy(toInsert->username, username, usernameLen+1);
	strncpy(toInsert->email, email, emailLen+1);
	strncpy(toInsert->passhash, passhash, passhashLen+1);

	return insertNode(toInsert);
}


/**
 * Insert user node into hash table
 * @param node The node to insert
 * @return 0 if successfully inserted, error code if not
 */
int AccountManager::insertNode(account_node_s* node)
{
	uint16_t hashPos = elfHash(reinterpret_cast<unsigned char*>(node->uid)) % _tableSize;

	account_node_s* slot = _table[hashPos];

	if(!slot) {
		_table[hashPos] = node;
		return 0;
	}

	while(slot->next != nullptr) {
		if(strcmp(slot->uid, node->uid) == 0) {
			return ERROR::DUPLICATE_ACCOUNT;
		}
		slot = slot->next;
	}

	if(strcmp(slot->uid, node->uid) == 0) {
		return ERROR::DUPLICATE_ACCOUNT;
	}

	slot->next = node; 

	return 0;
}



/**
 * Use PJW hash function to hash uids
 * @param s The string to hash
 * @return The hashed value of the string 
 */
uint32_t AccountManager::elfHash(const unsigned char* s)
{
	uint32_t h = 0, high;

	while(*s)
	{
		h = (h << 4) + *s++;
		if((high = h & 0xF0000000) != 0) {
			h ^= high >> 24;
		}
		h &= ~high;
	}

	return h;
}