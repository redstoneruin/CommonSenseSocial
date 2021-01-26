/**
 * Author: Ryan Steinwert
 * 
 * Header file for main Common Sense Social server class
 */

#define DEFAULT_BUF_SIZE 4096

#include <semaphore.h>
#include <stdint.h>

#include <openssl/ssl.h>

#include "CSDB/CSDBAccessManager.h"
#include "SessionManager.h"
#include "AccountManager.h"


typedef struct Thread {
    int cl;
    uint32_t session_id;
    sem_t mutex;
    SSL* ssl;
    char threadBuf[DEFAULT_BUF_SIZE];
} Thread;

class CSServer {
public:
    CSServer                            (int numCores);
    ~CSServer                           ();

    void startup                        ();


private:

    int _numThreads;
    uint16_t _port;

    sem_t _mutex;

    bool _shouldExit;

    Thread* _threadPool;

    // ssl context
    SSL_CTX* _ctx;

    CSDBAccessManager dbam; 
    SessionManager _sm;
    AccountManager _am;


    void* start                         (void* arg);

    void handleClient                   (Thread* thread);

    int parseMessage                    (Thread* thread);

    int readBytes                       (int cl, char* buf, uint16_t size);

    // command handlers
    void handleGetSessionID             (Thread* thread);
    void handleCreateAccount            (Thread* thread);
    void handleLogin                    (Thread* thread);

    // functions for ssl
    void initOpenSSL                    ();
    void cleanupOpenSSL                 ();
    SSL_CTX* createContext              ();
    void configureContext               (SSL_CTX* ctx);



    char* getCStr                       (const char* src, uint16_t size);
    char* getCStr                       (const char* src, uint16_t start, uint16_t size);

    uint64_t getInt                     (const char* src, uint16_t size);
    uint64_t getInt                     (const char* src, uint16_t start, uint16_t size);

    void placeInt                       (void* buf, uint64_t value, uint16_t start, uint16_t size);

};