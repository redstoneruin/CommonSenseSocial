/**
 * Author: Ryan Steinwert
 * 
 * Header file for main Common Sense Social server class
 */

#define DEFAULT_BUF_SIZE 4096

#include <semaphore.h>
#include <stdint.h>

#include <openssl/ssl.h>


typedef struct Thread {
    int cl;
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


    void* start                         (void* arg);

    void handleClient                   (Thread* thread);

    int readBytes                       (int cl, char* buf, uint16_t size);

    // functions for ssl
    void initOpenSSL                    ();
    void cleanupOpenSSL                 ();
    SSL_CTX* createContext              ();
    void configureContext               (SSL_CTX* ctx);



    char* getCStr                       (const char* src, uint16_t size);
    char* getCStr                       (const char* src, uint16_t start, uint16_t size);

    uint64_t getInt                     (const char* src, uint16_t size);
    uint64_t getInt                     (const char* src, uint16_t start, uint16_t size);

};