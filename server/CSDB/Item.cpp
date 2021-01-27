/**
 * Author: Ryan Steinwert
 * 
 * Implementation file for item class
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>

#include "Item.h"

using namespace std;


/**
 * Constructor for item type, sets instance variables and fills data field
 */
Item::Item(const char* name, const char* owner, PERM perm, DTYPE type, void* collection, const void* dataBuf, size_t dataSize) :
	Item(name, owner, perm, type, collection, dataSize)
{
	setData(dataBuf, dataSize);
}


/**
 * Constructor for item type with no data
 */
Item::Item(const char* name, const char* owner, PERM perm, DTYPE type, void* collection, size_t dataSize) :
   _perm(perm),
	_type(type),
	_loaded(false),
	_collection(collection),
	_dataSize(dataSize),
	_data(nullptr)
{
   if(name) _name = string(name);
   if(owner) _owner = string(owner);

	_createdTime = time(nullptr);
	_modifiedTime = time(nullptr);
}

Item::~Item()
{
	if(_data != nullptr) free(_data);
}


/**
 * Load item data from the file at given path
 * @param path The path to load file from
 * @return 0 if successful, error code if not
 */
int Item::load(const char* path)
{	
	int fd;
	size_t ret;
	void* data;

    fd = open(path, O_RDONLY);
    if(fd < 0) return ERROR::FILE_OPEN;

    data = (void*) malloc (sizeof(char) * _dataSize);

    if(_type == DTYPE::TEXT) {
        ret = read(fd, data, _dataSize-1);
        
        if(ret != _dataSize-1) {
            free(data);
            return ERROR::FILE_READ;
        }

        ((char*)data)[_dataSize-1] = 0;
    } else {
    	ret = read(fd, data, _dataSize);

    	if(ret != _dataSize) {
    		free(data);
    		return ERROR::FILE_READ;
    	}
    }

	_data = data;

    _loaded = true;

    return 0;
}

/**
 * Unload the item data from this item
 */
void Item::unload()
{
	if(_data != nullptr) free(_data);
	_loaded = false;
}


/**
 * Write this item to the file at a given path
 * @param path The path to write file to
 * @return 0 if successful, error code if not
 */
int Item::writeItem(const char* path)
{	
	int fd;

    // write to a file
    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT);


    if(fd < 0) return ERROR::FILE_OPEN;

    // determine how to write file based on type
    if(write(fd, _data, _dataSize) == -1) return ERROR::FILE_WRITE;

    close(fd);

    return 0;
}


/**
 * Set the item's data fields
 * @param dataBuf The data buffer to copy from
 * @param dataSize The size of the data to copy
 */
void Item::setData(const void* dataBuf, size_t dataSize)
{
	if(_data != nullptr) free(_data);
	// copy data from buffer into item
	_data = (void*) malloc (dataSize);

	memcpy(_data, dataBuf, dataSize);

	_loaded = true;
}

/**
 * Set the owner of this item
 * @param owner The new owner
 */
void Item::setOwner(const char* owner)
{
   _owner = string(owner);

}


// simple setters
void Item::setCollection(void* collection) 				{_collection = collection;}
void Item::setCreatedTime(time_t createdTime) 			{_createdTime = createdTime;}
void Item::setModifiedTime(time_t modifiedTime) 		{_modifiedTime = modifiedTime;}



// getters
string Item::name() 				{return _name;}
string Item::owner() 			{return _owner;}
PERM Item::perm() 				{return _perm;}
DTYPE Item::type()				{return _type;}
time_t Item::createdTime()		{return _createdTime;}
time_t Item::modifiedTime()		{return _modifiedTime;}
bool Item::loaded()				{return _loaded;}
void* Item::collection()		{return _collection;}
size_t Item::dataSize()			{return _dataSize;}
void* Item::data()				{return _data;}