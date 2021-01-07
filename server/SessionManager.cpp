/**
 * Author: Ryan Steinwert
 *
 * Implementation for server session manager
 */

#define DEFAULT_TABLE_SIZE 64

#include <cstdlib>
#include <ctime>

#include <random>

#include "SessionManager.h"




SessionManager::SessionManager(uint16_t tableSize) :
	_tableSize(tableSize)
{
	// create the table
	_table = (session_s**) malloc (sizeof(session_s*) * _tableSize);
	for(uint16_t i = 0; i < _tableSize; i++)
	{
		_table[i] = nullptr;
	}
}


SessionManager::SessionManager() : SessionManager(DEFAULT_TABLE_SIZE)
{
}


SessionManager::~SessionManager()
{
}


/**
 * Create and add new session to hash table
 * @return The id for the created session, 0 if could not create
 */
uint32_t SessionManager::createSession()
{
	uint32_t id;

	std::random_device r;

	std::default_random_engine e(r());

	std::uniform_int_distribution<uint32_t> uniform_dist(1, UINT32_MAX);

	id = uniform_dist(e);

	session_s* session = (session_s*) malloc (sizeof(session_s));
	session->id = id;
	session->uid = nullptr;
	session->next = nullptr;

	if(insert(session) != 0) return 0;

	return id;
}




/**
 * Insert a new session pointer into the session table
 * @param session The new session to insert
 * @return 0 if successfully inserted, error code if not
 */
int SessionManager::insert(session_s* session)
{
	uint16_t slotPos = hash(session->id);

	session_s* slot = _table[slotPos];

	if(!slot) {
		_table[slotPos] = session;
		return 0;
	}

	while(slot->next != nullptr)
	{
		if(slot->id == session->id) return ERROR::DUPLICATE_SESSION;
	}

	if(slot->id == session->id) return ERROR::DUPLICATE_SESSION;

	slot->next = session;

	return 0;
}

/**
 * Get session from the hash table
 * @param sessionId The id to look for
 * @return pointer to the session, null if does not exist
 */
session_s* SessionManager::getSession(uint32_t sessionId)
{
	session_s* slot = _table[hash(sessionId)];

	while(slot != nullptr)
	{
		if(slot->id == sessionId) break;
		slot = slot->next;
	}

	return slot;
}


/**
 * Create a hash to the table size based on session id
 * @param sessionId The session ID to hash
 * @return A valid table position hashed from input
 */
uint16_t SessionManager::hash(uint32_t sessionId)
{
	return sessionId % _tableSize;
}