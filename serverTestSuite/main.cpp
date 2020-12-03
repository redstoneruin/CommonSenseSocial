/**
 * Author: Ryan Steinwert
 * 
 * Test suite for Common Sense Social server
 */


#define DEFAULT_HOST_NAME "localhost"
#define DEFAULT_PORT 9251

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>


#include "definitions.h"


struct hostent *hent;
struct sockaddr_in addr;
int sock;

void printResult(int testResult);

int connectionTest1();

int headerTest1();


int main(int argc, char* argv[])
{
    uint16_t port = DEFAULT_PORT;

    // set up port
    hent = gethostbyname(DEFAULT_HOST_NAME);

    // set server settings
    memcpy(&addr.sin_addr.s_addr, hent->h_addr, hent->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);


    printf("Connection test: ");
    printResult(connectionTest1());

    printf("Header test 1: ");
    printResult(headerTest1());

    return 0;
}

int connectionTest1()
{
    return connect(sock, (struct sockaddr *)&addr, sizeof(addr));
}

int headerTest1()
{
    int result = send(sock, "testing", HEADER_SIZE, 0);
    if(result != HEADER_SIZE) {
        return -1;
    }
    return 0;
}


/**
 * Print success or FAILED based on given result of test
 */
void printResult(int testResult) 
{
    if(testResult == 0) {
        printf("success\n");
    } else {
        printf("FAILED: %d\n", testResult);
    }
}