/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Social main server class implementation file
 */

#define DEFAULT_PORT 9251
#define DEFAULT_SERVER_NAME "localhost"

#define BASE 256

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <thread>

#include <netdb.h>
#include <sys/socket.h>



#include "CSServer.h"
#include "definitions.h"


/**
 * Constructor for Common Sense Social server, starts main 
 * server loop.
 * numThreads - Number of threads in the thread pool
 */
CSServer::CSServer(int numThreads) :
    _numThreads(numThreads), 
    _port(DEFAULT_PORT),
    _shouldExit(false),
    _threadPool(nullptr)
{

    if(sem_init(&_mutex, 0, 0) != 0) err(2, "sem_init for main mutex");


    // initialize the thread pool
    _threadPool = (Thread*) malloc (sizeof(Thread) * _numThreads);

    for(int i = 0; i < _numThreads; i++) {
        // init mutex for each, set client to 0
        Thread* t = _threadPool + i;

        if(sem_init(&(t->mutex), 0, 0) != 0) err(2, "sem_init for thread");

        t->cl = 0;

        // start up this thread
        new std::thread(&CSServer::start, this, t);
    }
}


/**
 * CSServer deconstructor, clean up allocated resources
 */
CSServer::~CSServer()
{
    if(_threadPool != nullptr) free(_threadPool);
}



/**
 * Server startup, begins main server loop
 */
void CSServer::startup()
{
    // set up port
    struct hostent *hent = gethostbyname(DEFAULT_SERVER_NAME);
    struct sockaddr_in addr;

    // set server settings
    memcpy(&addr.sin_addr.s_addr, hent->h_addr, hent->h_length);
    addr.sin_port = htons(_port);
    addr.sin_family = AF_INET;

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    while(!_shouldExit) {
        // listen on socket for new clients
        listen(sock, 0);
        int cl = accept(sock, NULL, NULL);

        // look for available thread
        for(int i = 0; i < _numThreads; i++) {
            Thread* t = _threadPool + i;

            if(t->cl == 0) {
                // found available thread, set client and start
                t->cl = cl;
                if(sem_post(&(t->mutex)) != 0) err(2, "sem_post starting worker thread");
                break;
            }
        }
    }
}


/**
 * Function for starting thread, waits on semaphore before client is accepted
 * arg - Thread struct argument expected
 */
void* CSServer::start(void* arg)
{
    Thread* thread = (Thread*)arg;

    while(true) {
        if(sem_wait(&(thread->mutex)) != 0) err(2, "sem_wait on thread mutex");

        // handle client
        handleClient(thread);

        close(thread->cl);

        thread->cl = 0;
    }

    return 0;
}


/**
 * Handle client, called from worker thread when new client available
 * thread - thread responsible for handling this client
 */
void CSServer::handleClient(Thread* thread)
{

    printf("Handling client: %d\n", thread->cl);

    // continue reading command while connected
    while(true) 
    {
        size_t bytesRead;
        // Read from client until full tls record received
        if((bytesRead = read(thread->cl, thread->threadBuf, DEFAULT_BUF_SIZE)) == 0)
        {
            fprintf(stderr, "Client %d exited\n", thread->cl);
            return;
        }

        // pass bytes read to server
        //server.received_data((uint8_t*)thread->threadBuf, bytesRead);
        printf("Received %zu bytes from client %d\n", bytesRead, thread->cl);

    }
}


/* *
 * Read the specified number of bytes from the buffer client descriptor
 * up to the given maximum size
 * cl - client file descriptor for 
 */
int CSServer::readBytes(int cl, char* buf, uint16_t size)
{
    int rcvSize = 0;

    while(rcvSize < size) {
        int thisRcvSize = read(cl, (void*)((uint64_t)buf + rcvSize), size - rcvSize);
        rcvSize += thisRcvSize;
        //printf("Received %d bytes\n", thisRcvSize);

        if(thisRcvSize <= 0) return rcvSize;
    }

    return rcvSize;
}


/**
 * Returns new c string in heap from source buffer, starting at index 0 and with given size
 * src - source buffer to read string from
 * size - size of string
 */
char* CSServer::getCStr(const char* src, uint16_t size)
{
    return getCStr(src, 0, size);
}

/**
 * Returns a new c string stored in heap, with given start index and size from source buffer
 * src - source buffer to read string from
 * start - start index of string
 * size - size of string
 */
char* CSServer::getCStr(const char* src, uint16_t start, uint16_t size)
{
    char* dest = (char*) malloc (sizeof(char) * (size + 1));

    memcpy(dest, src+start, size);
    dest[size] = (char)0;

    return dest;
}


/**
 * Returns the parsed integer from a source buffer starting at index 0
 * src - buffer to read int from
 * size - size of int in bytes
 */
uint64_t CSServer::getInt(const char* src, uint16_t size)
{
    return getInt(src, 0, size);
}


/**
 * Returns the parsed integer from a given place in a source buffer
 * src - buffer to read int from
 * start - start index of int
 * size - size of int in bytes
 */
uint64_t CSServer::getInt(const char* src, uint16_t start, uint16_t size)
{
    int place = 0;
    uint64_t result = 0;
    // go from back to front, add based on powers of 8
    for(int i = (start+size)-1; i >= start; i--) {
        uint8_t val = (uint8_t)src[i];
        result += val * pow(BASE, place);
        place++;
    }

    return result;
}