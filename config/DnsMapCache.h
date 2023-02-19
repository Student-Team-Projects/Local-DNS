#pragma once

#include <mutex>
#include "DnsMap.h"
#include "DnsMapUser.h"

class DnsMapCache {
private:
    std::mutex m;
    #if GLOBAL
        const std::string filename = "/var/cache/local_dns/DnsMapCache.config";
    #else
        const std::string filename = "../config/DnsMapCache.config";
    #endif
    DnsMap dnsMap;
public:
    DnsMapCache();

    void updateEntry(const std::string &dns_name, const std::vector<std::string> &attributes);

    // returns {ip, timestamp} vector
    std::vector<std::string> getIpAttributes(const std::string &mac);

    void synchronizeCacheWithUserConfig(DnsMapUser& dnsMapUser);
};
