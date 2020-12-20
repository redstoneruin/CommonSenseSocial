/**
 * Author: Ryan Steinwert
 * 
 * Implementation file for TLS server credentials Botan extension
 */

#include "ServerCredentials.h"


ServerCredentials::ServerCredentials() 
    : m_key(Botan::PKCS8::load_key("sslcerts/67.180.255.189.key", rng))
{
}


std::vector<Botan::Certificate_Store*> ServerCredentials::trusted_certificate_authorities(
    const std::string& type,
    const std::string& context)
{
    // if client authentication is required, this function
    // shall return a list of certificates of CAs we trust
    // for tls client certificates, otherwise return an empty list
    return std::vector<Botan::Certificate_Store*>();
}

std::vector<Botan::X509_Certificate> ServerCredentials::cert_chain(
    const std::vector<std::string>& cert_key_types,
    const std::string& type,
    const std::string& context)
{
    // return the certificate chain being sent to the tls client
    return { Botan::X509_Certificate("sslcerts/67.180.255.189.crt") };
}


Botan::Private_Key* ServerCredentials::private_key_for(const Botan::X509_Certificate& cert,
    const std::string& type,
    const std::string& context)
{
    // return the private key associated with the leaf certificate,
    return m_key;
}