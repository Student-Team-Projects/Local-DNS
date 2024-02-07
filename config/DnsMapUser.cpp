#include "DnsMapUser.h"

void DnsMapUser::updateEntry(std::string const& dns_name, std::string const& mac) {
    dnsMap.updateEntry(dns_name, { mac });
}

std::string DnsMapUser::getMacFromDnsName(std::string const& dns_name) {
    auto attributes = dnsMap.getEntry(dns_name);
    if(attributes.empty()) {
        return "";
    }
    return attributes[0];
}

DnsMapUser::DnsMapUser() : dnsMap(filename) {
}

std::unordered_set<std::string> DnsMapUser::entries() {
    return dnsMap.entries();
}
