/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Database class definition
 */

enum PERM {
    PRIVATE,
    UNLISTED,
    PUBLIC
};

typedef struct collection_t {
    char* name;
    char* path;
    collection_t* subCollections;
} collection_s;

typedef struct item_t {
    char* name;
    char* owner;
    PERM perms;
} item_s;

class CSDB {
public:
    CSDB();
    CSDB(const char* dirname);
    ~CSDB();

    void loadDB(const char* filename);

private:
    const char* _dbDirname;

    void setup();
};