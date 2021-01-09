/**
 * Author: Ryan Steinwert
 * 
 * Account manager class definition
 */


#include <cstdint>

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

	int insertAccount(const char* uid, const char* username, const char* email, const char* passhash);

private:
	uint16_t _tableSize;
	account_node_s** _table;

	int insertNode(account_node_s* node);

	uint32_t elfHash(const unsigned char* ch);
};