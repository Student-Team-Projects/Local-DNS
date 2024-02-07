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
#include "dns_server_utils.h"
#include "tcp.h"
#include "udp.h"
#include <optional>

#include "../config/DnsMapUser.h"
#include "../config/DnsMapUserSettings.h"
#include "../networking/ip_getter.h"
#include "../networking/crafter_requester.h"
#include "../networking/utils.h"

int main(int argc, char** argv) {

    int c;
    int dns_port = 53;
    std::string dns_address = "127.0.0.1";
    std::string upstream_dns = "8.8.8.8";
    int upstream_port = 53;
    bool is_tcp = false;
    int timeout = 3000;
    std::string domain = "localdns";

    static struct option long_options[] = {
        { "port", required_argument, 0, 'p' },     { "address", required_argument, 0, 'a' },
        { "upstream", required_argument, 0, 'u' }, { "tcp", required_argument, 0, 't' },
        { "timeout", required_argument, 0, 'o' },  { "domain", required_argument, 0, 'd' }
    };
    while(1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "p:a:u:o:d", long_options, &option_index);

        /* Detect the end of the options. */
        if(c == -1)
            break;

        std::string pom;
        int val;
        switch(c) {
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
            domain = is_tcp = std::stoi(optarg);
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

    if(is_tcp) {
        pid_t pid = fork();
        if(pid == 0) {
            tcp(dns_port, dns_address, upstream_dns, upstream_port, timeout, domain);
        }
    }

    udp(dns_port, dns_address, upstream_dns, upstream_port, timeout, domain);
}
