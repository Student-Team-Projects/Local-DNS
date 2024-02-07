#include "DnsMapUserSettings.h"

DnsMapUserSettings::DnsMapUserSettings() : dnsMap(filename) {
}

std::vector<std::string> DnsMapUserSettings::get_settings(std::string const& setting_name) {
    return dnsMap.getEntry(setting_name);
}

std::string DnsMapUserSettings::get_setting(std::string const& setting_name) {
    return get_settings(setting_name)[0];
}
