/**
 * Author: Ryan Steinwert
 * 
 * Header file for TLS server credentials Botan extension
 */

#include <vector>
#include <string>
#include <memory>

#include <botan/credentials_manager.h>
#include <botan/certstor.h>
#include <botan/pk_keys.h>
#include <botan/pkcs8.h>
#include <botan/auto_rng.h>
#include <botan/tls_policy.h>


class ServerCredentials : public Botan::Credentials_Manager
{
public:
    ServerCredentials();

    std::vector<Botan::Certificate_Store*> trusted_certificate_authorities(
        const std::string& type,
        const std::string& context) override;

    std::vector<Botan::X509_Certificate> cert_chain(
        const std::vector<std::string>& cert_key_types,
        const std::string& type,
        const std::string& context) override;

    Botan::Private_Key* private_key_for(const Botan::X509_Certificate& cert,
        const std::string& type,
        const std::string& context) override;

private:
    Botan::AutoSeeded_RNG rng;
    Botan::Private_Key* m_key;
};