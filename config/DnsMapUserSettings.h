#pragma once

#include <string>
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

    std::string get_setting(std::string setting_name);
};
