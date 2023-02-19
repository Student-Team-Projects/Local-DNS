#pragma once

#include "DnsMap.h"

class DnsMapUser {
private:


    #if GLOBAL
        const std::string filename = "/etc/local_dns/DnsMapUser.config";
    #else
        const std::string filename = "../config/DnsMapUser.config";
    #endif
    DnsMap dnsMap;

public:
    DnsMapUser();

    void updateEntry(const std::string &dns_name, const std::string &mac);

    std::string getMacFromDnsName(const std::string &dns_name);

    std::unordered_set<std::string> entries();
};
