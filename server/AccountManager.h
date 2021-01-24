#pragma once
/**
 * Author: Ryan Steinwert
 * 
 * Account manager class definition
 */


#include <cstdint>
#include <random>
#include <vector>


typedef struct account_node_t {
	char* uid;
	char* username;
	char* email;
	char* passhash;
	account_node_t* next;
} account_node_s;

typedef struct account_info_t {
	char* username;
	char* email;
	char* uid;
} account_info_s;


class AccountManager {
public:
	AccountManager();
	AccountManager(uint16_t tableSize);
	~AccountManager();


	int createAccount(const char* username, const char* email, const char* password);
	int insertAccount(const char* uid, const char* username, const char* email, const char* passhash);
	int deleteAccount(const char* uid);

	int getUidFromUsername(const char* username, void* buf, size_t bufSize);
	int getUidFromEmail(const char* email, void* buf, size_t bufSize);

	bool accountExists(const char* uid);
	int getUsername(const char* uid, void* buf, size_t bufSize);

	account_info_s* login(const char* username, const char* password, int* error);

private:
	uint16_t _tableSize;
	account_node_s** _table;
	std::vector<account_info_s*> _infoList;

	std::random_device r;

	int insertNode(account_node_s* node);
	void freeNode(account_node_s* node);

	void loadAccounts();

	account_node_s* getNode(const char* uid);

	uint32_t elfHash(const unsigned char* ch);

	char* genRandString(int length);

	char* genHashString(const char* s);

	account_info_s* getAccountInfo(const char* username);

	bool matchPassWithHash(const char* password, const char* passhash);

	void writeNewAccount(account_node_s* account);
	void writeAccounts();
};