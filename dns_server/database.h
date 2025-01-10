#pragma once

#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <crafter.h>
#include <ctime>

struct addressesResponse {
    std::string RData;
    uint16_t RDataLength;
    time_t expired;
    bool local;

    addressesResponse() : RData(""), RDataLength(0), expired(0), local(false) {}
    addressesResponse(std::string RData, uint16_t RDataLength, time_t expired, bool local = false): RData(RData), RDataLength(RDataLength), expired(expired), local(local){}
};

extern std::unordered_map<std::string, std::unordered_map<uint32_t, std::vector<addressesResponse>>> addresses;

// Creates second key for addresses map from DNS Class and Type
uint32_t dnsMapKey(uint16_t DNSClass, uint16_t DNSType);

// Gets DNS Type from second key from addresses map
uint16_t dnsMapType(uint32_t key);

// Gets DNS Class from second key from addresses map
uint16_t dnsMapClass(uint32_t key);

// Apppends line containing DNS record to a file. Expire sets expiry data (timestamp)
void CSVAppend (std::string filename, Crafter::DNS::DNSAnswer dns, time_t expire, bool local);

// Apppends line containing DNS record to a file. Expire sets expiry data (timestamp)
void CSVAppendLocal (std::string filename, std::string name, addressesResponse dns);

// Read CSV file into addresses map
void CSVParse (std::string filename);