/**
 * Author: Ryan Steinwert
 * 
 * Account manager class definition
 */


typedef struct account_node_t {
	char* uid;
	char* username;
	char* email;
	char* passhash;
	account_node_t* left;
	account_node_t* right;
} account_node_s;


class AccountManager {
public:
	AccountManager();
	~AccountManager();

	int insertAccount(const char* uid, const char* username, const char* email, const char* passhash);

private:
	account_node_s* _root;

	int insertNode(account_node_s* node);
};