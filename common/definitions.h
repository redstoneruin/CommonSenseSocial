#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Common definitions for Common Sense Social server/client protocol
 */

/**
 * Server command identifiers
 */
#define GET_SESSION_ID 0x1001
#define CREATE_ACCOUNT 0x1002
#define LOGIN 0x1003
#define GET 0x2001

/**
 * Server command flags
 */
#define TEXT_RESOURCE 0x01
#define IMAGE_RESOURCE 0x02
#define AUDIO_RESOURCE 0x03
#define VIDEO_RESOURCE 0x04
#define STREAM_RESOURCE 0x05
#define AUDIO_STREAM_RESOURCE 0x06

/**
 * Server response identifiers
 */
#define SUCCESS 0x00
#define NOT_FOUND 0x01
#define NO_ACCESS 0x02


/**
 * Definitions for buffer sized
 */
#define HEADER_SIZE 6
#define IDENT_SIZE 4
#define COMMAND_SIZE 2
#define ERR_CODE_SIZE 2
#define STR_LEN_SIZE 2

#define MAX_COLLECTION_NAME_SIZE 64
#define MAX_ITEM_NAME_SIZE 64
#define MAX_LOGIN_FIELD_SIZE 128


enum ERROR {
	PARSE = 1,
	NO_PERMS = 2,
	PATH_INVAL = 3,
	PARAM_INVAL = 4,
	NO_DB = 5,
	FILE_OPEN = 6,
	FILE_READ = 7,
	FILE_WRITE = 8,
	PARENT_COLL_INVAL = 9,
	COLL_INVAL = 10,
	ITEM_CREATE = 11,
	NO_SESSION = 12,
	NO_ACCOUNT = 13,
	DUPLICATE_SESSION = 14,
	DUPLICATE_ACCOUNT = 15,
   BAD_LOGIN = 16,
   COMMAND_FORMAT = 17
};

enum PERM {
    PRIVATE,
    UNLISTED,
    PUBLIC
};

enum DTYPE {
    NONE,
    TEXT,
    IMAGE,
    AUDIO,
    VIDEO,
    STREAM,
    AUDIO_STREAM
};

/**
 * Struct for database request info
 */
typedef struct request_info_t {
	const char* uid;
	const char* perms;
	bool isAdmin = false;
} request_info_s;
