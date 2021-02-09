/**
 * Author: Ryan Steinwert
 * 
 * Test suite for Common Sense Social server
 */


#define DEFAULT_HOST_NAME "localhost"
#define DEFAULT_PORT 9251
#define DEFAULT_BUF_SIZE 2048

#define SHORT_BUF_SIZE 50

#define BASE 256

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include <iostream>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include "definitions.h"


using namespace std;

struct hostent *hent;
struct sockaddr_in addr;
uint8_t* buf;
int sock;

SSL* ssl;
uint32_t sessionID;


void printResult(int testResult);
uint64_t getInt(const char* src, uint16_t start, uint16_t size);
void placeInt(void* voidBuf, uint64_t value, uint16_t start, uint16_t size);


int headerTest1();
int loginTest1();
int createAccountTests();
int postTests();


int main(int argc, char* argv[])
{
   // ssl setup
   BIO* certBio;
   //BIO* outBio;
   X509* cert;
   X509_NAME* certName;
   const SSL_METHOD* method;
   SSL_CTX* ctx;

   // init openssl
   OpenSSL_add_all_algorithms();
   ERR_load_BIO_strings();
   ERR_load_crypto_strings();
   SSL_load_error_strings();

   // init bios
   certBio = BIO_new(BIO_s_file());
   //outBio = BIO_new_fp(stdout, BIO_NOCLOSE);

   // init ssl library
   if(SSL_library_init() < 0) {
      printf("Could not init OpenSSL library\n");
      return 1;
   }

   method = SSLv23_client_method();

   // create SSL context
   if((ctx = SSL_CTX_new(method)) == nullptr) {
      printf("Could not create ssl context");
      return 1;
   }

   //SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);

   // create ssl with context
   ssl = SSL_new(ctx);


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


   // bind ssl to file descriptor
   SSL_set_fd(ssl, sock);

   // attempt ssl connection
   if(SSL_connect(ssl) != 1) {
      printf("Could not build ssl session\n");
      return 1;
   } else {
      printf("SSL session built\n");
   }

   // get server cert
  // cert = SSL_get_peer_certificate(ssl);
  // if(cert == nullptr) {
  //    printf("Could not retrieve cert from server\n");
  //    return 1;
  // } else {
  //    printf("Received server cert\n");
  // }




  // size_t bytesRead;
  // while((bytesRead = read(sock, buf, DEFAULT_BUF_SIZE)) > 0) {
  //    // pass read bytes to client
  //    printf("received %zu bytes from server\n", bytesRead);
  // }


   printf("Header test 1: ");
   printResult(headerTest1());

   printf("Account creation tests: ");
   printResult(createAccountTests());

   printf("Login test 1: ");
   printResult(loginTest1());

   if(ssl != nullptr) SSL_free(ssl);
   close(sock);
   if(cert != nullptr) X509_free(cert);
   SSL_CTX_free(ctx);

   return 0;
}

int headerTest1()
{
   int written;
   char commandBuf[HEADER_SIZE];

   placeInt(commandBuf, 0, 0, 4);
   placeInt(commandBuf, CMD::GET_SESSION_ID, 4, 2);

   if((written = SSL_write(ssl, commandBuf, HEADER_SIZE)) <= 0) {
      return -1;
   }

   printf("Wrote %d bytes: ", written);


   int bytesRead = SSL_read(ssl, commandBuf, HEADER_SIZE);

   if(bytesRead < HEADER_SIZE) return -2;

   sessionID = getInt(commandBuf, 0, 4);

   printf("%#x: ", sessionID);

   return 0;
}


int createAccountTests()
{
  int written, bytesRead, err;
  char commandBuf[STR_LEN_SIZE+SHORT_BUF_SIZE];

  placeInt(commandBuf, sessionID, 0, IDENT_SIZE);
  placeInt(commandBuf, CMD::CREATE_ACCOUNT, IDENT_SIZE, COMMAND_SIZE);
  if(SSL_write(ssl, commandBuf, HEADER_SIZE) <= 0) return -1;


  placeInt(commandBuf, SHORT_BUF_SIZE, 0, STR_LEN_SIZE);
  strncpy(commandBuf+STR_LEN_SIZE, "myusername", SHORT_BUF_SIZE);
  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE+SHORT_BUF_SIZE) <= 0) return -2;

  strncpy(commandBuf+STR_LEN_SIZE, "myemail@gmail.com", SHORT_BUF_SIZE);
  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE+SHORT_BUF_SIZE) <= 0) return -3;

  strncpy(commandBuf+STR_LEN_SIZE, "password", SHORT_BUF_SIZE);
  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE+SHORT_BUF_SIZE) <= 0) return -4;



  bytesRead = SSL_read(ssl, commandBuf, HEADER_SIZE+ERR_CODE_SIZE);

  if(bytesRead < HEADER_SIZE+ERR_CODE_SIZE) return -5;

  err = static_cast<int>(getInt(commandBuf, HEADER_SIZE, ERR_CODE_SIZE));

  printf("Returned with [%d]: ", err);

  return 0;
}


int loginTest1()
{
  int bytesRead, err;
  char commandBuf[STR_LEN_SIZE+SHORT_BUF_SIZE];

  placeInt(commandBuf, sessionID, 0, IDENT_SIZE);
  placeInt(commandBuf, CMD::LOGIN, IDENT_SIZE, COMMAND_SIZE);
  if(SSL_write(ssl, commandBuf, HEADER_SIZE) <= 0) return -1;


  placeInt(commandBuf, SHORT_BUF_SIZE, 0, STR_LEN_SIZE);
  strncpy(commandBuf+STR_LEN_SIZE, "myusername", SHORT_BUF_SIZE);
  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE+SHORT_BUF_SIZE) <= 0) return -2;

  strncpy(commandBuf+STR_LEN_SIZE, "password", SHORT_BUF_SIZE);
  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE+SHORT_BUF_SIZE) <= 0) return -3;


  bytesRead = SSL_read(ssl, commandBuf, HEADER_SIZE+ERR_CODE_SIZE);

  if(bytesRead < HEADER_SIZE+ERR_CODE_SIZE) return -4;


  return static_cast<int>(getInt(commandBuf, HEADER_SIZE, ERR_CODE_SIZE));

}

int postTests()
{
  int bytesRead, err;
  char commandBuf[STR_LEN_SIZE+SHORT_BUF_SIZE];

  placeInt(commandBuf, sessionID, 0, IDENT_SIZE);
  placeInt(commandBuf, CMD::POST, IDENT_SIZE, COMMAND_SIZE);
  if(SSL_write(ssl, commandBuf, HEADER_SIZE) <= 0) return -1;

  strncpy(commandBuf, "w", 1);
  if(SSL_write(ssl, commandBuf, 1) <= 0) return -2;

  placeInt(commandBuf, SHORT_BUF_SIZE, 0, STR_LEN_SIZE);
  strncpy(commandBuf+STR_LEN_SIZE, "", SHORT_BUF_SIZE);

  if(SSL_write(ssl, commandBuf, STR_LEN_SIZE + SHORT_BUF_SIZE) <= 0) return -3;

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


/**
 * Returns the parsed integer from a given place in a source buffer
 * @param src Buffer to read int from
 * @param start Start index of int
 * @param size Size of int in bytes
 * @return The parsed int
 */
uint64_t getInt(const char* src, uint16_t start, uint16_t size)
{
    int place = 0;
    uint64_t result = 0;
    // go from back to front, add based on powers of 8
    for(int i = (start+size)-1; i >= start; i--) {
        result += static_cast<uint8_t>(src[i]) << 8*place;
        place++;
    }

    return result;
}


/**
 * Place int into the given buffer
 * @param buf Buffer to place into
 * @param value Value to place
 * @param start Start index for the int in the buffer
 * @param size Size in bytes of int to place
 */
void placeInt(void* voidBuf, uint64_t value, uint16_t start, uint16_t size)
{
    char* buffer = (char*)voidBuf;
    for(int32_t i = start+size-1; i >= start; --i)
    {
        buffer[i] = (char)(value & 0xFF);
        value = value >> 8; 
    }
}