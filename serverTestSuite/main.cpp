/**
 * Author: Ryan Steinwert
 * 
 * Test suite for Common Sense Social server
 */


#define DEFAULT_HOST_NAME "localhost"
#define DEFAULT_PORT 9251
#define DEFAULT_BUF_SIZE 2048

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>


#include "definitions.h"



struct hostent *hent;
struct sockaddr_in addr;
uint8_t* buf;
int sock;




//Botan::Processor_RNG rng;


void printResult(int testResult);


int headerTest1();

int loginTest1();


int main(int argc, char* argv[])
{

   int result;
   uint16_t port = DEFAULT_PORT;

   // create buffer
   buf = (uint8_t *) malloc (sizeof(uint8_t) * DEFAULT_BUF_SIZE);

   // set up port
   hent = gethostbyname(DEFAULT_HOST_NAME);

   // set server settings
   memcpy(&addr.sin_addr.s_addr, hent->h_addr, hent->h_length);
   addr.sin_port = htons(port);
   addr.sin_family = AF_INET;

   // create socket
   sock = socket(AF_INET, SOCK_STREAM, 0);

   result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

   if(result != 0) {
      printf("Could not connect to server, error code: %d\n", result);
      return 1;
   }


   size_t bytesRead;
   while((bytesRead = read(sock, buf, DEFAULT_BUF_SIZE)) > 0) {
      // pass read bytes to client
      printf("received %zu bytes from server\n", bytesRead);
   }



   printf("Header test 1: ");
   printResult(headerTest1());

   printf("Login test 1: ");
   printResult(loginTest1());

   return 0;
}

int headerTest1()
{
    //client->send("testing");

    return 0;
}

int loginTest1()
{
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