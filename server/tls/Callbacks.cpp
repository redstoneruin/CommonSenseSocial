/**
 * Author: Ryan Steinwert
 * 
 * Implementation for callbacks from Botan TLS server
 */

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "Callbacks.h"

Callbacks::Callbacks(int cl)
{
    _cl = cl;
}

void Callbacks::tls_emit_data(const uint8_t data[], size_t size)
{
    printf("Sending data with size: %zu\n", size);
    size_t result = send(_cl, data, size, 0);

    if(result != size) {
        printf("Could not send data to client %d, sent %zu out of %zu bytes\n", _cl, result, size);
    }
}


void Callbacks::tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size)
{
    printf("Received record of size: %zu\n", size);
}



void Callbacks::tls_alert(Botan::TLS::Alert alert)
{

}



bool Callbacks::tls_session_established(const Botan::TLS::Session& session)
{
    return false;
}
