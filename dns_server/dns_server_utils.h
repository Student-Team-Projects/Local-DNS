#pragma once

#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <crafter.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <fstream>
#include <string>
#include <getopt.h>
#include <optional>

#include "../config/DnsMapUser.h"
#include "../config/DnsMapUserSettings.h"
#include "../networking/ip_getter.h"
#include "../networking/crafter_requester.h"
#include "../networking/utils.h"

int constexpr DEFAULT_CACHE_TIMEOUT = 30;
std::string const DEFAULT_IP_MASK = "255.255.255.0";
#define BUFFER_SIZE 1024

std::string getDnsServerRedirect(std::string dnsServer, std::string upstream_dns);
static bool areValidIfaceFlags(int flags);
static bool isValidIface(std::string const& iface, int socketFd, ifreq* ifr);
static std::vector<std::string> filterValidIfaces(std::vector<std::string> const& ifaceNames);
static std::vector<std::string> scanForValidIfaces();
std::optional<std::string> getIface(DnsMapUserSettings& settings);
int getCacheTimeout(DnsMapUserSettings& settings);
std::optional<std::string> getMask(DnsMapUserSettings& settings, std::string const& iface);