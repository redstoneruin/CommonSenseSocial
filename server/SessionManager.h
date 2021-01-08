/**
 * Author: Ryan Steinwert
 *
 * Definition for server session manager class
 */

#include <vector>
#include <cstdint>

#include "definitions.h"


typedef struct session_t {
	uint32_t id;
	char* uid;
	session_t* next;
} session_s;



class SessionManager {
public:
	SessionManager();
	SessionManager(uint16_t tableSize);
	~SessionManager();

	uint32_t createSession();

	session_s* getSession(uint32_t sessionId);

	int deleteSession(uint32_t sessionId);

	int replaceUid(uint32_t sessionId, const char* uid);

private:
	uint16_t _tableSize;
	session_s** _table;

	uint16_t hash(uint32_t sessionId);

	int insert(session_s* session);

	void freeSession(session_s* session);
};
