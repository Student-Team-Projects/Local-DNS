#pragma once

#include <mutex>
#include "DnsMap.h"
#include "DnsMapUser.h"

class DnsMapCache {
private:
    std::mutex m;
#if GLOBAL
    std::string const filename = "/var/cache/local_dns/DnsMapCache.config";
#else
    std::string const filename = "../config/DnsMapCache.config";
#endif
    DnsMap dnsMap;

public:
    DnsMapCache();

    void updateEntry(std::string const& dns_name, std::vector<std::string> const& attributes);

    // returns {ip, timestamp} vector
    std::vector<std::string> getIpAttributes(std::string const& mac);

    void synchronizeCacheWithUserConfig(DnsMapUser& dnsMapUser);
};
