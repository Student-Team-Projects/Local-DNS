#pragma once

#include <string>
#include <vector>
#include "DnsMap.h"

class DnsMapUserSettings {
private:

    #if GLOBAL
        const std::string filename = "/etc/local_dns/DnsMapUserSettings.config";
    #else
        const std::string filename = "../config/DnsMapUserSettings.config";
    #endif
    DnsMap dnsMap;

public:
    DnsMapUserSettings();

    std::vector<std::string> get_settings(std::string const& setting_name);
    std::string get_setting(std::string const& setting_name);
};
