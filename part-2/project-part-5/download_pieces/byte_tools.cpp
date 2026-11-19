#include "byte_tools.h"
#include <openssl/sha.h>
#include <vector>

#include "byte_tools.h"

int BytesToInt(std::string_view bytes) {
    return (static_cast<unsigned char>(bytes[0]) << 24) |
           (static_cast<unsigned char>(bytes[1]) << 16) |
           (static_cast<unsigned char>(bytes[2]) << 8) |
           (static_cast<unsigned char>(bytes[3]));
}


std::string CalculateSHA1(const std::string& msg) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(msg.data()), msg.size(), hash);
    return std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
}
