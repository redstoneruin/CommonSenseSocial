#pragma once
/**
 * Author: Ryan Steinwert
 * 
 * Account manager class definition
 */


#include <cstdint>
#include <random>



typedef struct account_node_t {
	char* uid;
	char* username;
	char* email;
	char* passhash;
	account_node_t* next;
} account_node_s;


class AccountManager {
public:
	AccountManager();
	AccountManager(uint16_t tableSize);
	~AccountManager();


	int createAccount(const char* username, const char* email, const char* password);
	int insertAccount(const char* uid, const char* username, const char* email, const char* passhash);
	int deleteAccount(const char* uid);

	bool accountExists(const char* uid);
	int getUsername(const char* uid, void* buf, size_t bufSize);

private:
	uint16_t _tableSize;
	account_node_s** _table;

	std::random_device r;

	int insertNode(account_node_s* node);
	void freeNode(account_node_s* node);

	account_node_s* getNode(const char* uid);

	uint32_t elfHash(const unsigned char* ch);

	char* genRandString(int length);

	char* genHashString(const char* s);


	void writeNewAccount(account_node_s* account);
};