#pragma once

#include "DnsMap.h"

class DnsMapUser {
private:
#if GLOBAL
    std::string const filename = "/etc/local_dns/DnsMapUser.config";
#else
    std::string const filename = "../config/DnsMapUser.config";
#endif
    DnsMap dnsMap;

public:
    DnsMapUser();

    void updateEntry(std::string const& dns_name, std::string const& mac);

    std::string getMacFromDnsName(std::string const& dns_name);

    std::unordered_set<std::string> entries();
};
