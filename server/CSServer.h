/**
 * Author: Ryan Steinwert
 * 
 * Header file for main Common Sense Social server class
 */

#define DEFAULT_BUF_SIZE 4096

#include <semaphore.h>
#include <stdint.h>

typedef struct Thread {
    int cl;
    sem_t mutex;
    char threadBuf[DEFAULT_BUF_SIZE];
} Thread;

class CSServer {
public:
    CSServer                            (int numCores);
    ~CSServer                           ();

private:
    int _numThreads;
    uint16_t _port;

    sem_t _mutex;

    bool _shouldExit;

    Thread* _threadPool;


    void startup                        ();

    void* start                         (void* arg);

    void handleClient                   (Thread* thread);
};