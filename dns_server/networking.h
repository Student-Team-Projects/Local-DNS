#pragma once

#include <iostream>
#include <vector>
#include <crafter.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <unordered_map>

// Structure to store interface information
struct InterfaceInfo {
    std::string ip;
    std::string mac;
    std::string mask;
};

//returns all usable interfaces
std::unordered_map<std::string, InterfaceInfo> getAllInterfaces();

//from a given interface returns mac => ip
std::unordered_map<std::string, std::string> scanInterface(std::string name, InterfaceInfo iface);