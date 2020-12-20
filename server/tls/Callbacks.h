/**
 * Author.h
 * 
 * Header file for Botan TLS server callbacks
 */

#include <botan/tls_callbacks.h>

class Callbacks : public Botan::TLS::Callbacks
{
public:
    Callbacks(int cl);

    void tls_emit_data(const uint8_t data[], size_t size) override;

    void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) override;

    void tls_alert(Botan::TLS::Alert alert) override;

    bool tls_session_established(const Botan::TLS::Session& session) override;

private:
    int _cl;
};