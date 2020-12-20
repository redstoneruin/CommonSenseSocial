/**
 * Author: Ryan Steinwert
 * 
 * Common sense social server main file
 */

#define DEFAULT_NUM_THREADS 4

#include <stdio.h>
#include <getopt.h>

#include "CSServer.h"


int main(int argc, char* argv[]) {

    int opt;
    
    // use getopt
    while((opt = getopt(argc, argv, "")) != -1) {
        switch(opt) {
            default:
                break;
        }
    }


    CSServer* server = new CSServer(DEFAULT_NUM_THREADS);

    server->startup();

    delete server;
}