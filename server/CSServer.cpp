/**
 * Author: Ryan Steinwert
 * 
 * Common Sense Social main server class implementation file
 */

#define DEFAULT_PORT 9251
#define DEFAULT_SERVER_NAME "localhost"

#define BASE 256

#include <err.h>
#include <pthread.h>
#include <unistd.h>

#include <thread>
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <bitset>

#include <netdb.h>
#include <sys/socket.h>

#include <openssl/err.h>


#include "CSServer.h"
#include "definitions.h"

using namespace std;


/**
 * Constructor for Common Sense Social server, starts main 
 * server loop.
 * @param numThreads Number of threads in the thread pool
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
    int sock;

    initOpenSSL();

    // create and configure ssl context
    _ctx = createContext();
    configureContext(_ctx);
    
    // set up port
    struct hostent *hent = gethostbyname(DEFAULT_SERVER_NAME);
    struct sockaddr_in addr;

    // set server settings
    memcpy(&addr.sin_addr.s_addr, hent->h_addr, hent->h_length);
    addr.sin_port = htons(_port);
    addr.sin_family = AF_INET;

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);


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

    // cleanup on server close
    close(sock);
    SSL_CTX_free(_ctx);
    cleanupOpenSSL();
}

/**
 * OpenSSL initialization step
 */
void CSServer::initOpenSSL()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

/**
 * OpenSSL cleanup step
 */
void CSServer::cleanupOpenSSL()
{
    EVP_cleanup();
}

/**
 * Create SSL context
 * @return A new SSL context
 */
SSL_CTX* CSServer::createContext()
{
    const SSL_METHOD *method;
    SSL_CTX* ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

/**
 * Configure SSL context
 * @param ctx Pointer to SSL context to configure
 */
void CSServer::configureContext(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "sslcerts/certchain.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	    exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "sslcerts/key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	    exit(EXIT_FAILURE);
    }
}



/**
 * Function for starting thread, waits on semaphore before client is accepted
 * @param arg Thread struct argument expected
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
 * @param thread thread responsible for handling this client
 */
void CSServer::handleClient(Thread* thread)
{

    printf("Handling client: %d\n", thread->cl);

    // init ssl
    thread->ssl = SSL_new(_ctx);

    SSL_set_fd(thread->ssl, thread->cl);

    if (SSL_accept(thread->ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    }
    else {
        cout << "SSL accepted" << endl;
        //SSL_write(thread->ssl, reply, strlen(reply));
    }

    // continue reading command while connected
    while(true) 
    {
        int bytesRead;
        // Read from client until full tls record received
        if((bytesRead = SSL_read(thread->ssl, thread->threadBuf, HEADER_SIZE)) <= 0)
        {
            cout << "Client " << thread->cl << " exited\n";
            break;
        }

        // pass bytes read to server
        //server.received_data((uint8_t*)thread->threadBuf, bytesRead);
        if(bytesRead > 0) {            
            // parse message
            if(parseMessage(thread) != 0) {
                cerr << "Received invalid message from client" << endl;
                break;
            } 
        } 

    }

    SSL_shutdown(thread->ssl);
    SSL_free(thread->ssl);
}


/**
 * Function for parsing and handling messages from client
 * @param thread The thread managing the current client
 * @param message The message received from the client
 * @return 0 if successfully parsed and handled, error code if not
 */
int CSServer::parseMessage(Thread* thread)
{
    thread->session_id = getInt(thread->threadBuf, 0, 4);
    uint16_t command = getInt(thread->threadBuf, 4, 2);
    
    uint8_t flags = (command & 0x0FF0) >> 4;
    command = command & 0xF00F;

    cout << "Parsing command: 0x" << hex << command << dec << endl;

    switch(command) {
    case GET_SESSION_ID:
        handleGetSessionID(thread);
        break;
    case CREATE_ACCOUNT:
        handleCreateAccount(thread);
        break;
    case LOGIN:
        //handleLogin(thread);
        break;
    default:
        return -1;
    }

    return 0;
}


/**
 * Handle get session id command, establishes new session
 * @param thread Thread requesting session ID
 */
void CSServer::handleGetSessionID(Thread* thread)
{
    char returnBuf[HEADER_SIZE];
    thread->session_id = _sm.createSession();

    placeInt(returnBuf, thread->session_id, 0, IDENT_SIZE);
    placeInt(returnBuf, GET_SESSION_ID, IDENT_SIZE, COMMAND_SIZE);

    SSL_write(thread->ssl, returnBuf, HEADER_SIZE);
}


/**
 * Handle creating account by server command
 * @param thread Thread requesting account creation
 */
void CSServer::handleCreateAccount(Thread* thread)
{
    int err;
    session_s* session;

    char *username, *email, *password;

    char returnBuf[HEADER_SIZE+ERR_CODE_SIZE];

    err = 0;

    long nextStrSize;

    // get username string
    if(SSL_read(thread->ssl, thread->threadBuf, STR_LEN_SIZE) < STR_LEN_SIZE) {
        err = ERROR::COMMAND_FORMAT;
    }

    nextStrSize = getInt(thread->threadBuf, STR_LEN_SIZE);

    if(!err && SSL_read(thread->ssl, thread->threadBuf, nextStrSize) < nextStrSize) {
        err = ERROR::COMMAND_FORMAT;
    }

    username = getCStr(thread->threadBuf, nextStrSize);

    // get email string
    if(!err && SSL_read(thread->ssl, thread->threadBuf, STR_LEN_SIZE) < STR_LEN_SIZE) {
        err = ERROR::COMMAND_FORMAT;
    }

    nextStrSize = getInt(thread->threadBuf, STR_LEN_SIZE);

    if(!err && SSL_read(thread->ssl, thread->threadBuf, nextStrSize) < nextStrSize) {
        err = ERROR::COMMAND_FORMAT;
    }

    email = getCStr(thread->threadBuf, nextStrSize);

    // get password string
    if(!err && SSL_read(thread->ssl, thread->threadBuf, STR_LEN_SIZE) < STR_LEN_SIZE) {
        err = ERROR::COMMAND_FORMAT;
    }

    nextStrSize = getInt(thread->threadBuf, STR_LEN_SIZE);

    if(!err && SSL_read(thread->ssl, thread->threadBuf, nextStrSize) < nextStrSize) {
        err = ERROR::COMMAND_FORMAT;
    }

    password = getCStr(thread->threadBuf, nextStrSize);


    // check for valid params
    if(strlen(username) > MAX_LOGIN_FIELD_SIZE
        || strlen(email) > MAX_LOGIN_FIELD_SIZE
        || strlen(password) > MAX_LOGIN_FIELD_SIZE) {
        err = ERROR::COMMAND_FORMAT;
    }

    placeInt(returnBuf, thread->session_id, 0, IDENT_SIZE);
    placeInt(returnBuf, CREATE_ACCOUNT, IDENT_SIZE, COMMAND_SIZE);

    session = _sm.getSession(thread->session_id);
    if(!err && !session) err = ERROR::NO_SESSION;

    // if format error, return before making account
    if(err != 0) {
        placeInt(returnBuf, err, HEADER_SIZE, ERR_CODE_SIZE);
        SSL_write(thread->ssl, returnBuf, HEADER_SIZE+ERR_CODE_SIZE);
        free(username);
        free(email);
        free(password);
        return;
    }



    // create the account
    err = _am.createAccount(username, email, password);

    placeInt(returnBuf, err, HEADER_SIZE, ERR_CODE_SIZE);
    SSL_write(thread->ssl, returnBuf, HEADER_SIZE+ERR_CODE_SIZE);

    // cleanup
    free(username);
    free(email);
    free(password);
}


/**
 * Read the specified number of bytes from the buffer client descriptor 
 * up to the given maximum size
 * @param cl Client file descriptor
 * @param buf Buffer to read bytes into
 * @param size Max size to read into buffer
 * @return The number of bytes read to the given buffer
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
 * @param src Source buffer to read string from
 * @param size Size of string
 * @return A new string in the heap with the given size
 */
char* CSServer::getCStr(const char* src, uint16_t size)
{
    return getCStr(src, 0, size);
}

/**
 * Returns a new c string stored in heap, with given start index and size from source buffer
 * @param src Source buffer to read string from
 * @param start Start index of string
 * @param size Size of string
 * @return A new string in the heap with the given size
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
 * @param src Buffer to read int from
 * @param size Size of int in bytes
 * @return The parsed int
 */
uint64_t CSServer::getInt(const char* src, uint16_t size)
{
    return getInt(src, 0, size);
}


/**
 * Returns the parsed integer from a given place in a source buffer
 * @param src Buffer to read int from
 * @param start Start index of int
 * @param size Size of int in bytes
 * @return The parsed int
 */
uint64_t CSServer::getInt(const char* src, uint16_t start, uint16_t size)
{
    int place = 0;
    uint64_t result = 0;
    // go from back to front, add based on powers of 8
    for(int i = (start+size)-1; i >= start; --i) {
        result += src[i] << 8*place;
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
void CSServer::placeInt(void* buf, uint64_t value, uint16_t start, uint16_t size)
{
    char* buffer = (char*)buf;
    for(int32_t i = start+size-1; i >= start; --i)
    {
        buffer[i] = (char)(value & 0xFF);
        value = value >> 8; 
    }
}