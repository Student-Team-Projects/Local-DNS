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
#include "tcp.cpp"
#include "udp.cpp"
#include <optional>

#include "../config/DnsMapUser.h"
#include "../config/DnsMapUserSettings.h"
#include "../networking/ip_getter.h"
#include "../networking/crafter_requester.h"
#include "../networking/utils.h"

int main(int argc, char **argv) {


    int c;
    int dns_port = 53;
    std::string dns_address = "127.0.0.1";
    std::string upstream_dns = "8.8.8.8";
    int upstream_port = 53;
    bool is_tcp = false;
    int timeout = 3000;
    std::string domain = "localdns";

    static struct option long_options[] =
            {
                    {"port",     required_argument, 0, 'p'},
                    {"address",  required_argument, 0, 'a'},
                    {"upstream", required_argument, 0, 'u'},
                    {"tcp",      required_argument, 0, 't'},
                    {"timeout",  required_argument, 0, 'o'},
                    {"domain",   required_argument, 0, 'd'}
            };
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "p:a:u:o:d",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        std::string pom;
        int val;
        switch (c) {
            case 'p':
                dns_port = std::atoi(optarg);
                break;
            case 'a':
                dns_address = optarg;
                break;
            case 'u':
                pom = optarg;
                val = pom.find(":");
                upstream_dns = pom.substr(0, val);
                pom = pom.substr(val + 1);
                upstream_port = std::stoi(pom);
                break;
            case 'o':
                timeout = std::atoi(optarg);
                break;
            case 'd':
                domain = optarg;
                break;
            case 't':
                domain =
                is_tcp = std::stoi(optarg);
                break;
            default:
                break;
        }
    }
    printf("dns_port: %d\n", dns_port);
    printf("address: %s\n", dns_address.c_str());
    printf("upstream_dns: %s\n", upstream_dns.c_str());
    printf("upstream_port: %d\n", upstream_port);
    printf("is_tcp: %d\n", is_tcp);
    printf("timeout: %d\n", timeout);
    printf("domain: %s\n", domain.c_str());

    Crafter::InitCrafter();

    if (is_tcp) {
        pid_t pid = fork();
        if (pid == 0) {
            tcp(dns_port, dns_address, upstream_dns, upstream_port, timeout, domain);
        }
    }

    udp(dns_port, dns_address, upstream_dns, upstream_port, timeout, domain);
}

std::string getDnsServerRedirect(std::string dnsServer, std::string upstream_dns) {

    std::string input;
    std::ifstream inputStream("/etc/resolv.conf");
    std::string nameserver_indicator = "nameserver ";

    while (std::getline(inputStream, input)) {
        std::string prefix = input.substr(0, nameserver_indicator.size());
        if (prefix == nameserver_indicator) {
            std::string dns = input.substr(nameserver_indicator.size());
            if (dns != dnsServer) {
                return dns;
            }
        }
    }

    return upstream_dns;
}

static bool areValidIfaceFlags(int flags) {
    return (flags & IFF_UP) && !(flags & IFF_LOOPBACK) && !(flags & IFF_NOARP);
}

static bool isValidIface(std::string const& iface, int socketFd, ifreq* ifr) {
    if (iface.length() >= IFNAMSIZ)
        return false;
    std::memset(ifr, 0, sizeof(ifreq));
    strcpy(ifr->ifr_name, iface.c_str());
    if (ioctl(socketFd, SIOCGIFFLAGS, ifr) != 0)
        return false;
    return areValidIfaceFlags(ifr->ifr_flags);
}

static std::vector<std::string> filterValidIfaces(std::vector<std::string> const& ifaceNames) {
    // Here, we are creating a dummy socket
    // It will not really be used, its only purpose is to be passed to `ioctl` as a valid file descriptor
    int socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == -1) {
        std::cout << "Local DNS problem: Cannot create a socket to ensure the validity of the provided interfaces" << std::endl;
        return {};
    }

    ifreq ifr; // a common structure used for requests to ioctl

    std::vector<std::string> valid;
    for (auto const& name : ifaceNames)
        if (isValidIface(name, socketFd, &ifr))
            valid.emplace_back(name);

    close(socketFd);

    return valid;
}

static std::vector<std::string> scanForValidIfaces() {
    ifaddrs* ifaces;
    if (getifaddrs(&ifaces) != 0)
        return {};

    std::vector<std::string> valid;
    for (auto iface = ifaces; iface != nullptr; iface = iface->ifa_next) {
        if (iface->ifa_addr == nullptr)
            continue;
        if (iface->ifa_addr->sa_family == AF_INET && areValidIfaceFlags(iface->ifa_flags))
            valid.emplace_back(iface->ifa_name);
    }
    freeifaddrs(ifaces);
    return valid;
}

std::optional<std::string> getIface(DnsMapUserSettings& settings) {
    auto providedIfaces = settings.get_settings("iface");
    if (!providedIfaces.empty())
        std::cout << "Ifaces were provided in the config file, checking validity..." << std::endl;
    auto active = filterValidIfaces(providedIfaces);
    if (!active.empty()) {
        std::cout << "Choosing iface [" << active[0] << "] from the config file" << std::endl;
        return std::make_optional(active[0]);
    }
    std::cout << "No valid ifaces found in the config file; scanning..." << std::endl;
    auto scanned = scanForValidIfaces();
    if (!scanned.empty()) {
        std::cout << "Choosing iface [" << scanned[0] << "] from the scan" << std::endl;
        return std::make_optional(scanned[0]);
    }
    return std::nullopt;
}

int constexpr DEFAULT_CACHE_TIMEOUT = 30;

int getCacheTimeout(DnsMapUserSettings& settings) {
    auto providedTimeouts = settings.get_settings("cache_timeout");
    if (!providedTimeouts.empty()) {
        std::cout << "Cache timeout was provided in the config file, checking validity..." << std::endl;
        try {
            int result = std::stoi(providedTimeouts[0]);
            if (result >= 0) {
                std::cout << "Cache timeout successfully validated: " << result << std::endl;
                return result;
            }
            std::cout << "Cache timeout was negative, defaulting" << std::endl;
        } catch(std::invalid_argument) {
            std::cout << "Cache timeout argument was invalid, defaulting" << std::endl;
        } catch(std::out_of_range) {
            std::cout << "Cache timeout argument was out of range, defaulting" << std::endl;
        }
    } else
        std::cout << "No cache timeout was provided in the config file, defaulting" << std::endl;
    std::cout << "Default cache timeout: " << DEFAULT_CACHE_TIMEOUT << std::endl;
    return DEFAULT_CACHE_TIMEOUT;
}

std::string const DEFAULT_IP_MASK = "255.255.255.0";

std::optional<std::string> getMask(DnsMapUserSettings& settings, std::string const& iface) {
    auto masks = settings.get_settings("ip_mask");
    // Here, we are creating a dummy socket
    // It will not really be used, its only purpose is to be passed to `ioctl` as a valid file descriptor
    int socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == -1)
        return std::nullopt;
    ifreq ifr;
    std::string ifaceMask;
    std::memset(&ifr, 0, sizeof(ifreq));
    strcpy(ifr.ifr_name, iface.c_str());
    if (ioctl(socketFd, SIOCGIFNETMASK, &ifr) == 0)
        masks.emplace_back(ifr.ifr_netmask.sa_data);
    close(socketFd);
    masks.emplace_back(DEFAULT_IP_MASK);
    return getShortestValidMask(masks);
}
