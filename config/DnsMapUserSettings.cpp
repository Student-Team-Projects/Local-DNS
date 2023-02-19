#include "DnsMapUserSettings.h"

DnsMapUserSettings::DnsMapUserSettings(): dnsMap(filename) {

}

std::string DnsMapUserSettings::get_setting(std::string setting_name){
    return dnsMap.getEntry(setting_name)[0];
}
