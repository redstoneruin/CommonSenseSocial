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

#include <botan/tls_client.h>
#include <botan/tls_callbacks.h>
#include <botan/tls_session_manager.h>
#include <botan/tls_policy.h>
#include <botan/auto_rng.h>
#include <botan/certstor.h>
#include <botan/certstor_system.h>


#include "definitions.h"



struct hostent *hent;
struct sockaddr_in addr;
uint8_t* buf;
int sock;


/**
 * @brief Callbacks invoked by TLS::Channel.
 *
 * Botan::TLS::Callbacks is an abstract class.
 * For improved readability, only the functions that are mandatory
 * to implement are listed here. See src/lib/tls/tls_callbacks.h.
 */
class Callbacks : public Botan::TLS::Callbacks
{
   public:
      void tls_emit_data(const uint8_t data[], size_t size) override
      {
         // send data to tls server, e.g., using BSD sockets or boost asio
         printf("Sending data with size: %zu\n", size); 
         size_t result = send(sock, data, size, 0);
         if(result != size) {
            printf("Could not send data, sent %zu bytes out of %zu\n", result, size);
         }
      }

      void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) override
      {
         // process full TLS record received by tls server, e.g.,
         // by passing it to the application

         printf("Data received with size: %zu\n", size);
      }

      void tls_alert(Botan::TLS::Alert alert) override
      {
         // handle a tls alert received from the tls server
      }

      bool tls_session_established(const Botan::TLS::Session& session) override
      {
         // the session with the tls server was established
         // return false to prevent the session from being cached, true to
         // cache the session in the configured session manager
         return false;
      }
};

/**
 * @brief Credentials storage for the tls client.
 *
 * It returns a list of trusted CA certificates from a local directory.
 * TLS client authentication is disabled. See src/lib/tls/credentials_manager.h.
 */
class Client_Credentials : public Botan::Credentials_Manager
   {
   public:
      Client_Credentials()
         {
         // Here we base trust on the system managed trusted CA list
         //m_stores.push_back(new Botan::System_Certificate_Store);
         }

      std::vector<Botan::Certificate_Store*> trusted_certificate_authorities(
         const std::string& type,
         const std::string& context) override
         {
         // return a list of certificates of CAs we trust for tls server certificates
         // ownership of the pointers remains with Credentials_Manager
         return m_stores;
         }

      std::vector<Botan::X509_Certificate> cert_chain(
         const std::vector<std::string>& cert_key_types,
         const std::string& type,
         const std::string& context) override
         {
         // when using tls client authentication (optional), return
         // a certificate chain being sent to the tls server,
         // else an empty list
         return std::vector<Botan::X509_Certificate>();
         }

      Botan::Private_Key* private_key_for(const Botan::X509_Certificate& cert,
         const std::string& type,
         const std::string& context) override
         {
         // when returning a chain in cert_chain(), return the private key
         // associated with the leaf certificate here
         return nullptr;
         }

   private:
       std::vector<Botan::Certificate_Store*> m_stores;
};



//Botan::Processor_RNG rng;


void printResult(int testResult);


int headerTest1(Botan::TLS::Client* client);

int loginTest1(Botan::TLS::Client* client);


int main(int argc, char* argv[])
{

   Callbacks callbacks;
   Botan::AutoSeeded_RNG rng;
   Botan::TLS::Session_Manager_In_Memory session_mgr(rng);
   Client_Credentials creds;
   Botan::TLS::Default_Policy policy;

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

   // open the tls connection
   Botan::TLS::Client client(callbacks,
                              session_mgr,
                              creds,
                              policy,
                              rng,
                              Botan::TLS::Server_Information("67.180.255.189", DEFAULT_PORT),
                              Botan::TLS::Protocol_Version::TLS_V12);


   size_t bytesRead;
   while((bytesRead = read(sock, buf, DEFAULT_BUF_SIZE)) > 0) {
      // pass read bytes to client
      printf("received %zu bytes, need %zu more for full record\n", bytesRead, client.received_data(buf, bytesRead));
   }



   printf("Header test 1: ");
   printResult(headerTest1(&client));

   printf("Login test 1: ");
   printResult(loginTest1(&client));

   return 0;
}

int headerTest1(Botan::TLS::Client* client)
{
    client->send("testing");

    return 0;
}

int loginTest1(Botan::TLS::Client* client)
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