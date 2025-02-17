#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <ctime>

#include "database.h"
#include "networking.h"

using json = nlohmann::json;

struct settings {
    std::string iface;
    time_t timeout;
    InterfaceInfo ifaceInfo;
};

extern settings globalSettings;
extern std::unordered_map<std::string, std::string> macToDomain;
extern std::string CSVFile;


// Updates addresses with what was provided in DnsMapUser.config
bool updateLocalDNS();

// Generates globalSettings and calls updateLocalDNS()
void config();

// Prints globalSettings
void printSettings();

// From former dns_server_utils; Gets upstream server consulting /etc/resolv.conf
std::string getDnsServerRedirect(std::string dnsServer, std::string upstream_dns);
