/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Social main server class implementation file
 */

#define DEFAULT_PORT 9251
#define DEFAULT_SERVER_NAME "localhost"

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <thread>

#include <netdb.h>
#include <sys/socket.h>


#include "CSServer.h"


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

    startup();
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

        printf("Accepted client: %d\n", cl);

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
        handleClient(thread->cl);

        thread->cl = 0;
    }

    return 0;
}


/**
 * Handle client, called from worker thread when new client available
 * cl - client file descriptor
 */
void CSServer::handleClient(int cl)
{
    printf("Handling client: %d\n", cl);
}