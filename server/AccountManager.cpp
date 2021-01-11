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
 * Create a new account with random uid the hash of the given password, DOES NOT STORE PASSWORD
 * @param username The username for the user
 * @param email Email of the user
 * @param password Password for the user, hashed before storing
 */
int AccountManager::createAccount(const char* username, const char* email, const char* password)
{
	return 0;
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
 * Delete the account associated with the uid
 * @param uid The uid of the user to delete
 * @return 0 if successfully deleted, 0 if not
 */
int AccountManager::deleteAccount(const char* uid)
{
	uint16_t hashPos = elfHash(reinterpret_cast<const unsigned char*>(uid)) % _tableSize;

	account_node_s* prev = nullptr;
	account_node_s* slot = _table[hashPos];

	if(slot == nullptr) return ERROR::NO_ACCOUNT;

	while(slot != nullptr) {
		if(strcmp(slot->uid, uid) == 0) break;
		prev = slot;
		slot = slot->next;
	}

	if(slot == nullptr) return ERROR::NO_ACCOUNT;

	if(prev == nullptr) {
		_table[hashPos] = slot->next;
		freeNode(slot);
		return 0;
	}

	prev->next = slot->next;
	freeNode(slot);

	return 0;
}


/**
 * Get the username associated with a uid
 * @param uid The uid to get username for
 * @param buf The buffer to copy to
 * @param bufSize Max size to copy to buffer
 * @return 0 if successful, error code if not
 */
int AccountManager::getUsername(const char* uid, void* buf, size_t bufSize)
{
	account_node_s* node = getNode(uid);

	if(!node) return ERROR::NO_ACCOUNT;

	strncpy((char*)buf, node->username, bufSize);

	return 0;
}


/**
 * Returns whether the account exists
 * @param uid The uid of the acccount
 * @return True if exists, false if not
 */
bool AccountManager::accountExists(const char* uid)
{
	return getNode(uid);
}


/**
 * Return the node associated with the given user id
 * @param uid The user id
 * @return The pointer to the account node, null if does not exist
 */
account_node_s* AccountManager::getNode(const char* uid)
{
	uint16_t hashPos = elfHash(reinterpret_cast<const unsigned char*>(uid)) % _tableSize;

	account_node_s* slot = _table[hashPos];

	if(!slot) return nullptr;

	while(slot != nullptr) {
		if(strcmp(slot->uid, uid) == 0) return slot;
		slot = slot->next;
	}

	return nullptr;
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
			freeNode(node);
			return ERROR::DUPLICATE_ACCOUNT;
		}
		slot = slot->next;
	}

	if(strcmp(slot->uid, node->uid) == 0) {
		freeNode(node);
		return ERROR::DUPLICATE_ACCOUNT;
	}

	slot->next = node; 

	return 0;
}



/**
 * Free the memory associated with a node
 * @param node The node to free
 */
void AccountManager::freeNode(account_node_s* node)
{
	if(!node) return;

	if(node->uid) free(node->uid);
	if(node->username) free(node->username);
	if(node->email) free(node->email);
	if(node->passhash) free (node->passhash);
	free(node);
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