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


struct hostent *hent;
struct sockaddr_in addr;
int sock;


int connectionTest1();


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

    connectionTest1();
}

int connectionTest1()
{
    printf("Connecting to server...\n");
    
    return connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    return 0;
}