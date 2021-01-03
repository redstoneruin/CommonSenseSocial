#pragma once

/**
 * Author: Ryan Steinwert
 * 
 * Common definitions for Common Sense Social server/client protocol
 */

/**
 * Server command identifiers
 */
#define GET 0x1001

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

#define MAX_COLLECTION_NAME_SIZE 64
#define MAX_ITEM_NAME_SIZE 64



enum ERROR {
	PARSE,
	NO_PERMS,
	PATH_INVAL,
	PARAM_INVAL,
	NO_DB,
	FILE_OPEN,
	FILE_READ,
	FILE_WRITE,
	PARENT_COLL_INVAL,
	COLL_INVAL,
	ITEM_CREATE
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
