/**
 * Author: Ryan Steinwert
 *
 * Implementation file for CSDB collection tree
 */

#define COLLECTIONS_FILENAME "collections"
#define FORMATTED_COLLECTIONS_FILENAME "formattedCollections"


#define COLLECTIONS_CHILDREN_SIZE 4
#define COLLECTIONS_ITEMS_SIZE 8


#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "CollectionTree.h"

#include "../definitions.h"



CollectionTree::CollectionTree() : 
	_numBaseCollections(0),
	_collections(nullptr)
{
}



CollectionTree::~CollectionTree()
{
}



void CollectionTree::setup(const char* dirname)
{
	_dirname = dirname;

    std::string collFilename(_dirname);
    std::string formattedCollFilename(_dirname);
    collFilename.push_back('/');
    formattedCollFilename.push_back('/');
    collFilename.append(COLLECTIONS_FILENAME);
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);;

    // attempt to load formatted collections first
    if(loadTree(formattedCollFilename.c_str()) == 0) {
        // formatted collection file found
    } else if(loadTree(collFilename.c_str(), O_CREAT) == 0) {
        // plain collection file found, do formatting
        createFormattedCollectionsFile(formattedCollFilename.c_str());
    } else {
        fprintf(stderr, "Error: Could not find or create collections file\n");
        exit(1);
    }
}


/**
 * Load DB using collections file
 * @param collsFilename Filename for collections
 * @param extraFlags Extra flags for dealing with file opening
 * @return 0 if successful, error code if not
 */
int CollectionTree::loadTree(const char* collsFilename, unsigned int extraFlags)
{
    int fd;
    FILE* file;
    std::vector<collection_s*> baseCollections;
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + COLLECTIONS_CHILDREN_SIZE + 2];

    fd = open(collsFilename, O_RDWR | extraFlags);

    // should return without error, without message if file not found 
    if(fd == -1) {
        return ERROR::FILE_OPEN;
    }

    // open file in read only mode
    file = fdopen(fd, "r");
    if(file == nullptr) {
        fprintf(stderr, "Could not open collections file stream\n");
        return ERROR::FILE_OPEN;
    }

    // read through file parsing by whitespace
    while(fscanf(file, "%s", collNameBuf) == 1) {
        // parse the collection using helper function
        collection_s* parent = parseCollectionString(collNameBuf, nullptr);
        
        setupCollectionManifest(parent);
        
        collectionLoadHelper(file, parent);
        baseCollections.push_back(parent);
    }

    close(fd);


    // write the collections
    _numBaseCollections = baseCollections.size();
    _collections = (collection_s**) malloc (sizeof(collection_s*) * _numBaseCollections);
    memcpy(_collections, baseCollections.data(), _numBaseCollections * sizeof(collection_s*));


    return 0;
}



/**
 * Set up directory structure and ensure manifest file created for collection
 * @param coll Collection to set up
 */
void CollectionTree::setupCollectionManifest(collection_s* collection)
{
    int fd, colonIndex;
    FILE* file;
    char buf[MAX_COLLECTION_NAME_SIZE + COLLECTIONS_ITEMS_SIZE + 2];
    std::string manifestName(collection->path);
    manifestName.push_back('/');
    manifestName.append("Manifest");

    colonIndex = 0;

    // create dir
    mkdir(collection->path, S_IRWXU);

    // attempt to open manifest file
    fd = open(manifestName.c_str(), O_RDWR | O_CREAT);

    if(fd < 0) {
        fprintf(stderr, "Error: Could not open manifest file for collection: %s\n", collection->name);
        return;
    }

    file = fdopen(fd, "r+");    

    if(file == nullptr) {
        fprintf(stderr, "Error: Could not open file descriptor for collection: %s\n", collection->name);
        return;
    }

    if(fscanf(file, "%s", buf) < 1) {
        // write size 0 to manifest
        fprintf(file, "size:0 ");
        collection->numItems = 0;
        return;

    }

    // manifest exists, size in buffer
    for(int i = 0; i < MAX_COLLECTION_NAME_SIZE + 1; i++) {
        if(buf[i] == ':') {
            colonIndex = i;
            break;
        }
    }

    if(colonIndex == 0) {
        fprintf(stderr, "Error: Could not find collection size for collection: %s\n", collection->name);
    }

    // set num items to whats in the manifest
    collection->numItems = atoll(buf + colonIndex + 1);

    collection->items = (item_s**) malloc (sizeof(item_s*) * collection->numItems);


    // TODO: add items to collection
    for(unsigned long long i = 0; i < collection->numItems; i++)
    {
        if(fscanf(file, "%s", buf) < 1) {
            // not enough items
            fprintf(stderr, "Error: Not enough items in manifest at path: %s, buffer: %s\n", manifestName.c_str(), buf);
            fprintf(stderr, "Expected %llu items, got %llu\n", collection->numItems, i);
            exit(1);
        }


        // parse the buffer
        item_s* item = (item_s*) malloc (sizeof(item_s));

        item->loaded = false;
        item->collection = collection;

        // break buf into seperate strings at seperators
        for(unsigned long j = 0; j < sizeof(buf); j++) 
        {
            if(buf[j] == ':') {
                buf[j] = 0;
            } else if (buf[j] == 0) {
                break;
            }
        }

        int len, startIndex;

        len = strlen(buf);

        item->name = (char*) malloc (sizeof(char) * (len + 1));
        strncpy(item->name, buf, len);
        item->name[len] = 0;

        startIndex = len + 1;

        len = strlen(buf + startIndex);

        if(len == 0) {
            item->owner = nullptr;
        } else {
            item->owner = (char*) malloc (sizeof(char) * (len + 1));
            strncpy(item->owner, buf + startIndex, len);
            item->owner[len] = 0;
        }

        startIndex += len + 1;

        len = strlen(buf + startIndex);
        item->perm = (PERM)atoi(buf + startIndex);

        startIndex += len + 1;

        len = strlen(buf + startIndex);
        item->type = (DTYPE)atoi(buf + startIndex);

        startIndex += len + 1;

        len = strlen(buf + startIndex);
        item->createdTime = atol(buf + startIndex);

        startIndex += len + 1;

        len = strlen(buf + startIndex);
        item->modifiedTime = atol(buf + startIndex);

        startIndex += len + 1;
        item->dataSize = atol(buf + startIndex);

        collection->items[i] = item;

    }

    fclose(file);
    close(fd);

}


/**
 * Create formatted collections file
 * @param formattedCollFilename Filename for formatted collection file
 */
void CollectionTree::createFormattedCollectionsFile(const char* formattedCollFilename)
{
    FILE* file;
    if((file = fopen(formattedCollFilename, "w+")) == nullptr) {
        fprintf(stderr, "Could not create formatted collection file\n");
        return;
    }

    for(int i = 0; i < _numBaseCollections; i++) {
        formattedCollectionsHelper(file, _collections[i]);
    }

    fclose(file);
}


/**
 * Recursive helper for writing collections to formatted file
 * @param file File stream to write to, must b open
 * @param parent Parent collection
 */
void CollectionTree::formattedCollectionsHelper(FILE* file, collection_s* parent)
{
    // print parent
    fprintf(file, "%s:%d ", parent->name, parent->numSubColls);

    for(int i = 0; i < parent->numSubColls; i++) 
    {
        formattedCollectionsHelper(file, parent->subCollections[i]);
    }
}


/**
 * Add a collection with the given path to the collection structure
 * @param path Path for new collection
 * @return 0 if successful, error code if not
 */
int CollectionTree::addCollection(const char* path)
{

    size_t lastSepIndex;
    std::string pathstring(path);
    std::string formattedCollFilename(_dirname);

    if(!validCollectionPath(path)) return ERROR::PATH_INVAL;

    // check if collection already exists
    if(getCollection(path) != nullptr) return 0;

    formattedCollFilename.push_back('/');
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);

    lastSepIndex = pathstring.find_last_of('/');


    if(lastSepIndex == std::string::npos) {
        // add new base collection
        collection_s* newColl = parseCollectionString(pathstring.append(":0").c_str(), nullptr);

        collection_s** newCollections = (collection_s**) malloc (sizeof(collection_s*) * _numBaseCollections + 1);

        // copy over base collections
        for(int i = 0; i < _numBaseCollections; i++)
        {
            newCollections[i] = _collections[i];
        }

        newCollections[_numBaseCollections] = newColl;

        if(_collections != nullptr) free(_collections);
        _collections = newCollections;
        _numBaseCollections++;

        createFormattedCollectionsFile(formattedCollFilename.c_str());

        setupCollectionManifest(newColl);

        return 0;

    }
    
    std::string parentPath = pathstring.substr(0, lastSepIndex);
    std::string name = pathstring.substr(lastSepIndex + 1);
    std::string collstring = pathstring.substr(lastSepIndex + 1);
    collstring.append(":0");


    collection_s* parent = getCollection(parentPath.c_str());

    // ensure parent exists
    if(parent == nullptr) return ERROR::PARENT_COLL_INVAL;

    // look for child already existing
    for(int i = 0; i < parent->numSubColls; i++) {
        collection_s* child = parent->subCollections[i];
        // if name found, can return without failure
        if(strcmp(child->name, name.c_str()) == 0) return 0;
    }


    collection_s* child = parseCollectionString(collstring.c_str(), parent);

    // create new subcollection array and copy over old one
    collection_s** subCollections = (collection_s**) malloc (sizeof(collection_s*) * (parent->numSubColls + 1));
    memcpy(subCollections, parent->subCollections, sizeof(collection_s*) * parent->numSubColls);
    
    // add new child
    subCollections[parent->numSubColls] = child;

    // free the old subcollection list and replace
    free(parent->subCollections);
    parent->subCollections = subCollections;
    parent->numSubColls++;

    createFormattedCollectionsFile(formattedCollFilename.c_str());

    setupCollectionManifest(child);

    return 0;
}


/**
 * Delete the collection at the given path
 * @param path Path of collections to delete
 * @return 0 if deleted successfully, error code if not
 */
int CollectionTree::deleteCollection(const char* path)
{
    collection_s* toDelete = getCollection(path);

    if(toDelete == nullptr) return ERROR::PATH_INVAL;

    // delete from parent list
    if(toDelete->parent == nullptr) {
        // find position in base collections
        for(int i = 0; i < _numBaseCollections; i++) 
        {
            if(_collections[i] == toDelete) {
                // remove collection from list
                collection_s** collections = (collection_s**) malloc (sizeof(collection_s*) * (_numBaseCollections -1));
                // copy still valid colls
                for(int j = 0; j < _numBaseCollections; j++) 
                {
                    if(j == i) continue;

                    if(j > i) {
                        collections[j-1] = _collections[j];
                    } else {
                        collections[j] = _collections[j];
                    }
                }

                free(_collections);
                _collections = collections;
                _numBaseCollections--;
                break;
            }
        }
    } else {
        // remove from parent's child collections
        collection_s* parent = toDelete->parent;

        for(int i = 0; i < parent->numSubColls; i++)
        {
            if(parent->subCollections[i] == toDelete) {
                collection_s** subCollections = (collection_s**) malloc (sizeof(collection_s*) * (parent->numSubColls-1));
                
                for(int j = 0; j < parent->numSubColls; j++)
                {
                    if(j==i) continue;
                    if(j > i) {
                        subCollections[j-1] = parent->subCollections[j]; 
                    } else {
                        subCollections[j] = parent->subCollections[j];
                    }
                }
                free(parent->subCollections);
                parent->subCollections = subCollections;
                parent->numSubColls--;
                break;
            }
        }
    }

    deleteCollectionHelper(toDelete);

    std::string formattedCollFilename(_dirname);
    formattedCollFilename.push_back('/');
    formattedCollFilename.append(FORMATTED_COLLECTIONS_FILENAME);

    // rewrite collections file
    createFormattedCollectionsFile(formattedCollFilename.c_str());

    return 0;
}



/**
 * Find the collection with the given path, returns null if does not exist
 * @param path Path string for the collection
 * @return The collection for the given path, null if DNE
 */
collection_s* CollectionTree::getCollection(const char* path)
{
    int pathDepth;
    std::string pathstring(path);
    size_t nextSep, lastSep;
    std::vector<char*> collectionNames;
    collection_s *nextParent, *lastParent, *ret;

    if(!validCollectionPath(path)) return nullptr;

    ret = nullptr;
    
    // first break path into pieces
    nextSep = lastSep = -1;
    while((nextSep = pathstring.find('/', lastSep + 1)) != std::string::npos) 
    {
        int nameLen = nextSep - lastSep - 1;
        char* name = (char*) malloc (sizeof(char) * (nameLen + 1));

        memcpy(name, path + lastSep + 1, nameLen);
        name[nameLen] = 0;

        collectionNames.push_back(name);
        
        lastSep = nextSep;
    }

    // get the last 
    int nameLen = strlen(path + lastSep + 1);
    char* name = (char*) malloc (sizeof(char) * (nameLen + 1));
    memcpy(name, path + lastSep + 1, nameLen);
    name[nameLen] = 0;
    collectionNames.push_back(name);

    pathDepth = collectionNames.size();

    // attempt to find the collection
    for(int i = 0; i < _numBaseCollections; i++) 
    {
        nextParent = _collections[i];

        // match base collection name
        if(strcmp(nextParent->name, collectionNames[0]) != 0) {
            continue;
        } else if(pathDepth == 1) {
            ret = nextParent;
            break;
        }

        for(int j = 1; j < pathDepth; j++) 
        {
            lastParent = nextParent;
            // check each child, update
            for(int k = 0; k < nextParent->numSubColls; k++)
            {
                collection_s* child = lastParent->subCollections[k];
                if(strcmp(child->name, collectionNames[j]) == 0) {
                    // found next collection
                    nextParent = child;
                    break;
                }
            }

            if(lastParent == nextParent) {
                // not found under this parent
                break;
            } else if(j == pathDepth-1) {
                // matching child found at the max depth, must be result
                ret = nextParent;
                break;
            }
        }
        if(ret != nullptr) break;
    }

    // cleanup
    for(int i = 0; i < pathDepth; i++) {
        free(collectionNames[i]);
    }

    return ret;

}


/**
 * Add an item with the given path to the collection
 * @param path Path of the new item
 * @param text Test to store in this item
 * @return 0 if successful, error code if not
 */
int CollectionTree::replaceItem(const char* path, const char* text, const char* owner, PERM perm)
{
    size_t textLen;
    item_s* item;
    int ret;

    if(!validItemPath(path)) return -1;
    
    textLen = strlen(text);

    item = getNewItemStruct(path, owner, perm);

    if(item == nullptr) {
        return ERROR::ITEM_CREATE;
    }

    // set type and copy over test
    item->type = DTYPE::TEXT;
    item->data = (void*) malloc (sizeof(char) * (textLen + 1));
    
    strncpy((char*)item->data, text, textLen + 1);

    item->dataSize = textLen + 1;

    if((ret = addItemToParent(item)) != 0) {
        if(item->data != nullptr) free(item->data);
        free(item);
        return ret;
    }

    if((ret = writeItem(item)) != 0) return ret;

    return 0;

}


/**
 * Replace an item with a generic type
 * @param path The path of the item
 * @param data The buffer for data to store in the item
 * @param dataSize The size in bytes of the data to store
 * @param type The type of item
 * @param owner The owner of the item
 * @param perm The permission status of this item
 * @return 0 if succesfully replaced, error code if not
 */
int CollectionTree::replaceItem(const char* path, const void* data, size_t dataSize, DTYPE type, const char* owner, PERM perm)
{
    int ret;
    item_s* item;

    if(!validItemPath(path)) return -1;

    item = getNewItemStruct(path, owner, perm);

    if(item == nullptr) {
        return ERROR::ITEM_CREATE;
    }

    item->type = type;
    item->data = (void*) malloc (dataSize);
    item->dataSize = dataSize;

    memcpy(item->data, data, dataSize);

    if((ret = addItemToParent(item)) != 0) {
        if(item->data != nullptr) free(item->data);
        free(item);
        return ret;        
    }

    if((ret = writeItem(item)) != 0) return ret;

    return 0;
}


/**
 * Delete the item at the given path
 * @param path The path of the item
 * @return 0 if successfully deleted, error code if not
 */
int CollectionTree::deleteItem(const char* path)
{
    if(!validItemPath(path)) return ERROR::PATH_INVAL;

    item_s* item = getItem(path);

    if(item == nullptr) return ERROR::PATH_INVAL;

    collection_s* collection = (collection_s*)item->collection;

    if(collection == nullptr) return ERROR::PATH_INVAL;

    unsigned long long itemIndex;

    item_s** oldList = collection->items;
    item_s** newList = (item_s**) malloc (sizeof(item_s*) * (collection->numItems - 1));

    for(unsigned long long i = 0; i < collection->numItems; i++)
    {
        item_s* currentItem = collection->items[i];
        if(strcmp(currentItem->name, item->name) == 0)
        {
            itemIndex = i;
            break;
        }
    }

    // fill new list
    for(unsigned long long i = 0; i < collection->numItems; i++)
    {
        if(i == itemIndex) {
            continue;
        } else if(i > itemIndex) {
            newList[i-1] = oldList[i];
        } else {
            newList[i] = oldList[i];
        }
    }

    collection->items = newList;
    collection->numItems--;


    // delete file
    std::string filePathString(_dirname);
    filePathString.push_back('/');
    filePathString.append(path);

    // not checking if succeeds or not
    remove(filePathString.c_str());

    // free memory
    free(oldList);
    free(item->name);
    if(item->owner != nullptr) free(item->owner);
    free(item);

    updateManifest(collection);


    return 0;
}

/**
 * Get item struct from the given path
 * @param path The path of the item
 * @return The pointer to the item if exists, null if does not
 */
item_s* CollectionTree::getItem(const char* path)
{
    size_t sepIndex;
    collection_s* collection;
    std::string pathString(path);

    if(!validItemPath(path)) return nullptr;

    sepIndex = pathString.find_last_of('/');

    std::string nameString = pathString.substr(sepIndex + 1);
    std::string collPathString = pathString.substr(0, sepIndex);

    // first find the collection
    collection = getCollection(collPathString.c_str());

    if(collection == nullptr) return nullptr;


    return getItemFromCollection(collection, nameString.c_str());
}

/**
 * Get an item from the given collection
 * @param collection The collection to search
 * @return The pointer to the item if exists, nullif does not
 */
item_s* CollectionTree::getItemFromCollection(collection_s* collection, const char* name)
{
    for(unsigned long long i = 0; i < collection->numItems; i++)
    {
        item_s* item = collection->items[i];

        if(strcmp(item->name, name) == 0) return item;
    }

    return nullptr;
}



/**
 * Get the owner of the item at a given path
 * @param path The path of the item
 * @param ownerBuf Buffer to store owner string in
 * @param bufSize The maximum size of the buffer to write into
 * @return 0 if successfully wrote owner, error code if error
 */
int CollectionTree::getOwner(const char* path, void* buf, size_t bufSize)
{
    item_s* item = getItem(path);

    if(item == nullptr) return ERROR::PATH_INVAL;

    char* owner = item->owner;

    if(owner == nullptr) {
        strncpy((char*)buf, "", bufSize);
        return 0;
    }

    strncpy((char*)buf, owner, bufSize);

    return 0;
}


/**
 * Get the permissions for an item
 * @param path The path of the item
 * @param permPointer Pointer to perm item to store result
 * @return 0 if successful, error code if not
 */
int CollectionTree::getPerm(const char* path, PERM* permPointer)
{
    item_s* item = getItem(path);

    if(item == nullptr) return ERROR::PATH_INVAL;

    *permPointer = item->perm;

    return 0;
}


/**
 * Get item data from the item at a given path
 * @param path The path of the item
 * @param returnBuffer Buffer to write item data to
 * @param type Pointer to item type, filled with the type of the returned item data
 * @param bufSize The maximum size to write to the buffer
 * @param offset The offset to open the file with
 * @return The number of bytes written
 */
size_t CollectionTree::getItemData(const char* path, void* returnBuffer, DTYPE* type, size_t bufSize, size_t offset)
{
    item_s* item;
    char* buf = (char*)returnBuffer;

    if(!validItemPath(path)) return 0;

    // get the item
    item = getItem(path);

    if(item == nullptr) return 0;

    // load item into memory
    if(!item->loaded && loadItem(item) != 0) return 0;

    if(offset >= item->dataSize) return 0;

    size_t i;
    for(i = offset; i < item->dataSize && (i-offset) < bufSize; i++) 
    {
        buf[i-offset] = ((char*)item->data)[i];
    }

    *type = item->type;

    return offset-i;
}

/**
 * Load item data from file structure into memory
 * @param item Item to load
 * @return 0 if successful, error code if not
 */
int CollectionTree::loadItem(item_s* item)
{
    int fd;
    size_t ret;
    void* data;
    collection_s* parent = (collection_s*)item->collection;

    if(item == nullptr) return 0;

    if(item->loaded && item->data != nullptr) free(item->data);

    std::string pathString(parent->path);
    pathString.push_back('/');
    pathString.append(item->name);

    fd = open(pathString.c_str(), O_RDONLY);
    if(fd < 0) return ERROR::FILE_OPEN;

    data = (void*) malloc (sizeof(char) * item->dataSize);

    if(item->type == DTYPE::TEXT) {
        ret = read(fd, data, item->dataSize-1);
        
        if(ret != item->dataSize-1) {
            free(data);
            return ERROR::FILE_READ;
        }

        ((char*)data)[item->dataSize-1] = 0;
        item->data = data;
    }

    item->loaded = true;

    return 0; 
}


/**
 * Unload an item from memory
 * @param item The item to unload
 */
void CollectionTree::unloadItem(item_s* item)
{
    if(item == nullptr) return;

    if(!item->loaded) return;

    if(item->data != nullptr) free(item->data);

}



/**
 * Get and fill a new item struct with the given path
 * @param path The path of the item
 * @return 0 if successful, error code if not
 */
item_s* CollectionTree::getNewItemStruct(const char* path, const char* owner, PERM perm)
{
    std::string pathstring(path);
    std::string namestring;
    collection_s* collection;
    item_s* item;
    size_t lastSep;

    lastSep = pathstring.find_last_of('/');
    if(lastSep == std::string::npos) {
        return nullptr;
    }

    collection = getCollection(pathstring.substr(0, lastSep).c_str());

    if(collection == nullptr) {
        return nullptr;
    }

    // create the new item struct
    item = (item_s*) malloc (sizeof(item_s));

    namestring = pathstring.substr(lastSep+1);

    // copy over name    
    item->name = (char*) malloc (sizeof(char) * (namestring.size()+1));
    strncpy(item->name, namestring.c_str(), namestring.size()+1);

    item->owner = nullptr;

    // copy over owner if exists    
    if(owner != nullptr) {
        int ownerLen = strlen(owner);
        item->owner = (char*) malloc (sizeof(char) * (ownerLen + 1));
        strncpy(item->owner, owner, ownerLen+1);
    }

    item->perm = perm;

    item->loaded = false;
    item->dataSize = 0;
    item->data = nullptr;

    item->collection = collection;

    item->createdTime = time(nullptr);
    item->modifiedTime = time(nullptr);

    return item;
}



/**
 * Adds the given item to the list of its parent, if not there already
 * @param item Item to add to its parent collection
 * @return 0 if successfully added, error code if not
 */
int CollectionTree::addItemToParent(item_s* item)
{
    collection_s* parent = (collection_s*)item->collection;

    if(parent == nullptr) return ERROR::PATH_INVAL;


    std::string itemPath(parent->path);
    itemPath.push_back('/');
    itemPath.append(item->name);


    // check if already in list
    for(unsigned long long i = 0; i < parent->numItems; i++) 
    {
        item_s* currentItem = parent->items[i];
        if(currentItem == item) {
            return 0;
        } else if (strcmp(currentItem->name, item->name) == 0) {
            parent->items[i] = item;
            free(currentItem);
            return 0;
        }

    }

    // add to list
    item_s** oldlist = parent->items;
    item_s** newlist = (item_s**) malloc (sizeof(item_s*) * (parent->numItems + 1));
    
    for(unsigned long long i = 0; i < parent->numItems; i++) 
    {
        newlist[i] = oldlist[i];
    }
    newlist[parent->numItems] = item;

    if(oldlist != nullptr) free(oldlist);

    parent->items = newlist;
    parent->numItems++;


    return 0;
}


/**
 * Write an item to the file structure
 * @param item The item to write
 * @return 0 if successful, error code if not
 */
int CollectionTree::writeItem(item_s* item) 
{
    int ret;
    int fd;

    // update manifest first
    if((ret = updateManifest((collection_s*)item->collection)) != 0) return ret;

    collection_s* collection = (collection_s*)item->collection;

    if(collection == nullptr) return ERROR::PATH_INVAL;

    // get the path for the item
    std::string itemPath(collection->path);
    itemPath.push_back('/');
    itemPath.append(item->name);

    // write to a file
    fd = open(itemPath.c_str(), O_WRONLY | O_TRUNC | O_CREAT);


    if(fd < 0) return ERROR::FILE_OPEN;

    // determine how to write file based on type
    if(item->type == DTYPE::TEXT) {
        if(write(fd, item->data, item->dataSize) == -1) return ERROR::FILE_WRITE;
    }

    close(fd);

    return 0;
}


/**
 * Update the manifest for the current collection based on the items
 * @param collection The collection to update
 * @return 0 if successful, error code if not
 */
int CollectionTree::updateManifest(collection_s* collection)
{
    int manFile;
    
    if(collection == nullptr) return ERROR::COLL_INVAL;

    std::string manifestPathString(collection->path);
    manifestPathString.push_back('/');
    manifestPathString.append("Manifest");

    manFile = open(manifestPathString.c_str(), O_WRONLY | O_TRUNC);

    if(manFile < 0) return ERROR::FILE_OPEN;

    dprintf(manFile, "size:%llu", collection->numItems);

    // print info for each item
    for(unsigned long long i = 0; i < collection->numItems; i++) 
    {
        item_s* item = collection->items[i];

        dprintf(manFile, " %s:%s:%d:%d:%ld:%ld:%lu",  item->name, 
                                                    item->owner == nullptr ? "" : item->owner, 
                                                    item->perm, 
                                                    item->type,
                                                    item->createdTime,
                                                    item->modifiedTime,
                                                    item->dataSize);
    }

    close(manFile);

    return 0;
}




/**
 * Dumps the collection structure to the given file
 * @param file File to dump collection list
 */
void CollectionTree::dumpCollections(FILE* file)
{
    fprintf(file, "------------- Collection structure -------------\n");
    for(int i = 0; i < _numBaseCollections; i++) {
        dumpCollectionsHelper(file, _collections[i]);
    }
    fprintf(file, "------------------------------------------------\n");
}

/**
 * Helper function for dumping collections to file stream recursively
 * @param file File stream to dump to
 * @param parent Parent collection to print first
 * @param depth Current recursion depth, defaults to 0
 */
void CollectionTree::dumpCollectionsHelper(FILE* file, collection_s* parent, int depth)
{
    for(int i = 0; i < depth; i++) {
        fprintf(file, "\t");
    }

    fprintf(file, "%s: %s: %d sub-collections: %llu items\n", parent->name, parent->path, parent->numSubColls, parent->numItems);

    for(int i = 0; i < parent->numSubColls; i++) {
        dumpCollectionsHelper(file, parent->subCollections[i], depth+1);
    }
}

/**
 * Recursive helper function for filling collections structs
 * @param file File stream to read collections from
 * @param parent Current parent collection
 */
void CollectionTree::collectionLoadHelper(FILE* file, collection_s* parent)
{
    char collNameBuf[MAX_COLLECTION_NAME_SIZE + 1];

    setupCollectionManifest(parent);

    for(int i = 0; i < parent->numSubColls; i++) {
        if(fscanf(file, "%s", collNameBuf) != 1) {
            printf("Error loading collections, expected extra subcollection\n");
            exit(1);
        }

        collection_s* child = parseCollectionString(collNameBuf, parent);

        // recursive call
        collectionLoadHelper(file, child);

        parent->subCollections[i] = child;
    
    }
}

/**
 * Recursive helper for deleting collection
 * @param toDelete Collection to delete
 */
void CollectionTree::deleteCollectionHelper(collection_s* toDelete)
{
    // recursively delete children   
    for(int i = 0; i < toDelete->numSubColls; i++) 
    {
        deleteCollectionHelper(toDelete->subCollections[i]);
    }

    // delete items
    for(unsigned long long i = 0; i < toDelete->numItems; i++)
    {
        // TODO: free item info stored in struct
        free(toDelete->items[i]);
    }

    if(toDelete->items != nullptr) free(toDelete->items);


    // delete the directory in the filesystem
    nftw64(toDelete->path, ftwHelper, 50, FTW_DEPTH);

    free(toDelete);
}

/**
 * Helper function for ftw
 * @param path The path of the object to handle
 * @param statStruct The stat struct for the given object
 * @param info Extra info defined by ftw functions
 * @param ftw The ftw struct
 * @return Returns 0, undefined behavior
 */
int CollectionTree::ftwHelper(const char* path, const struct stat64*, int info, struct FTW*)
{
    if(info == FTW_DP) {
        // remove empty directory
        rmdir(path);
    } else if(info == FTW_F) {
        remove(path);
    }
    return 0;
}

/**
 * Parse a collection string
 * @param collectionString C string containing name and number of subcollections to parse
 * @param parent Parent collection, can be null
 * @return New collections struct in heap mem containing info from string
 */
collection_s* CollectionTree::parseCollectionString(const char* collectionString, collection_s* parent)
{
    // find ':' to parse the number of subcollections
    int colPos, nameLen;
    collection_s* newColl = (collection_s*) malloc (sizeof(collection_s));
    newColl->parent = parent;
    newColl->numItems = 0;
    newColl->path = nullptr;
    newColl->subCollections = nullptr;
    newColl->items = nullptr;
    
    nameLen = strlen(collectionString);
    for(int i = 0; i < nameLen; i++) {
        if(collectionString[i] == ':') {
            colPos = i;
        }
    }

    const char* numSubCollsString = collectionString + colPos + 1;

    // fill collection struct
    newColl->numSubColls = atoi(numSubCollsString);
    newColl->name = (char*) malloc (sizeof(char) * (colPos + 1));
    strncpy(newColl->name, collectionString, colPos);
    newColl->name[colPos] = 0;

    // set path based on parent
    std::string pathstring;

    if(parent == nullptr) {
        pathstring.append(_dirname);
        pathstring.push_back('/');
        pathstring.append(newColl->name);
    } else {
        // create path string
        pathstring.append(parent->path);
        pathstring.push_back('/');
        pathstring.append(newColl->name);
    }

    // set path string
    int len = pathstring.length(); 
    newColl->path = (char*) malloc (sizeof(char) * (len + 1));
    strncpy(newColl->path, pathstring.c_str(), len);

    // ensure null termination
    newColl->path[len] = 0;

    // allocate space for subcolls if exist
    if(newColl->numSubColls > 0) {
        newColl->subCollections = (collection_s**) malloc (sizeof(collection_s*) * newColl->numSubColls);
    }

    return newColl;


}







/**
 * Check whether the path string could be a valid collection
 * @param path The path to check
 * @return True if valid, false if not
 */
bool CollectionTree::validCollectionPath(const char* path)
{
    size_t length;
    char lastChar = 0;

    length = strlen(path);

    if(length == 0 || path[0] == '/') return false;

    for(size_t i = 0; i < length; i++) 
    {
        char c = path[i];
        // check characters are valid
        if(!validPathCharacter(c)){
            return false;
        } else if(c == '/' && lastChar == '/') {
            return false;
        }
        lastChar = c;
    }

    return true;
}



/**
 * Check if the path could be a valid item
 * @param path The item path to check
 * @param True if path valid, false if not
 */
bool CollectionTree::validItemPath(const char* path)
{
    if(!validCollectionPath(path)) return false;

    size_t length = strlen(path);

    for(size_t i = 0; i < length; i++) {
        if(path[i] == '/') return true;
    }

    return false;
}


/**
 * Return whether a character can be used in a valid path
 * @param c The char to check
 * @return True if the char can be validly used in a path, false if not
 */
bool CollectionTree::validPathCharacter(char c)
{

    if(    (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || (c == '/')
        || (c == '.')
        || (c == '+'))
            return true;
        
    return false;
}