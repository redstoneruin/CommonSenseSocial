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