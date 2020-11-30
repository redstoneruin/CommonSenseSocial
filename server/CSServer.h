/**
 * Author: Ryan Steinwert
 * 
 * Header file for main Common Sense Social server class
 */

class CSServer {
public:
    CSServer(int numCores);
    ~CSServer();

private:
    int _numThreads;
};