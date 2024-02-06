#include "utils.h"

#include <optional>
#include <set>
#include <string>
#include <vector>

#include <cstdlib>
#include <cstdint>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

std::set<std::string> getIPs(std::string const& baseIP, std::string const& maskDotted) {
    auto mask = dottedToBinary(maskDotted).value_or(0);
    auto baseAddress = dottedToBinary(baseIP).value_or(0) & mask;
    char textAddress[INET_ADDRSTRLEN];
    std::set<std::string> ips;
    for(uint32_t offset = 1; offset < (~mask); offset++) {
        uint32_t ip = htonl(baseAddress + offset);
        inet_ntop(AF_INET, &ip, textAddress, INET_ADDRSTRLEN);
        ips.emplace(textAddress);
    }
    return ips;
}

std::optional<uint32_t> dottedToBinary(std::string const& address) {
    uint32_t result;
    if(inet_pton(AF_INET, address.c_str(), &result) <= 0)
        return std::nullopt;
    return std::make_optional(ntohl(result));
}

std::optional<uint32_t> hostBitsOf(std::string const& mask) {
    auto binary = dottedToBinary(mask);
    return binary.has_value() ? hostBitsOf(binary.value()) : std::nullopt;
}

std::optional<uint32_t> hostBitsOf(uint32_t mask) {
    for(uint32_t i = 2; i <= 8; i++)
        if((~mask) == (1u << i) - 1u)
            return std::make_optional(i);
    return std::nullopt;
}

std::optional<std::string> getShortestValidMask(std::vector<std::string> const& masks) {
    std::string shortest;
    uint32_t len = MAX_HOST_BITS + 1; // bigger than the max so that we know whether there were any valid masks
    for(auto const& mask : masks) {
        auto host = hostBitsOf(mask);
        if(!host.has_value())
            continue;
        auto hostBits = host.value();
        if(hostBits < len) {
            len = hostBits;
            shortest = mask;
        }
    }
    return len <= MAX_HOST_BITS ? std::make_optional(shortest) : std::nullopt;
}
