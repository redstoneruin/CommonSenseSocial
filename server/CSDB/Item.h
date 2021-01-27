/**
 * Author: Ryan Steinwert
 *
 * Class definition for database items
 */

#include <ctime>
#include <string>

#include "../definitions.h"

class Item {
public:
	Item(const char* name, const char* owner, PERM perm, DTYPE type, void* collection, const void* dataBuf, size_t dataSize);
	Item(const char* name, const char* owner, PERM perm, DTYPE type, void* collection, size_t dataSize);

	~Item();


	int load(const char* path);
	void unload();

	int writeItem(const char* path);

	void setCollection(void* collection);
	void setData(const void* dataBuf, size_t dataSize);
	void setOwner(const char* owner);
	void setCreatedTime(time_t createdTime);
	void setModifiedTime(time_t modifiedTime);

	std::string name();
	std::string owner();
	PERM perm();
	DTYPE type();
	time_t createdTime();
	time_t modifiedTime();
	bool loaded();
	void* collection();
	size_t dataSize();
	void* data();

private:
	std::string _name;
	std::string _owner;
	PERM _perm;
	DTYPE _type;
	time_t _createdTime;
	time_t _modifiedTime;
	bool _loaded;
	void* _collection;
	size_t _dataSize;
	void* _data;
};