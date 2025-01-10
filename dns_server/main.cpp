#include <cstdio>
#include <getopt.h>

#include "udp.h"

int main(int argc, char** argv) {

    int c;
    int dns_port = 53;
    std::string dns_address = "127.0.0.1";
    std::string upstream_dns = "8.8.8.8";
    int upstream_port = 53;
    int timeout = 3000;
    std::string domain = "localdns";
    std::string csv_database = "/var/cache/local-dns/DnsDatabase.csv";
    bool debug = false;

    static struct option long_options[] = {
        { "port", required_argument, 0, 'p' },     { "address", required_argument, 0, 'a' },
        { "upstream", required_argument, 0, 'u' },
        { "timeout", required_argument, 0, 'o' },  { "domain", required_argument, 0, 'd' },
        { "csv_database", required_argument, 0, 'c' },  { "debug", required_argument, 0, 'D' }
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
        case 'c':
            csv_database = optarg;
            break;
        case 'D':
            debug = (bool)std::atoi(optarg);
            break;
        default:
            break;
        }
    }
    printf("dns_port: %d\n", dns_port);
    printf("address: %s\n", dns_address.c_str());
    printf("upstream_dns: %s\n", upstream_dns.c_str());
    printf("upstream_port: %d\n", upstream_port);
    printf("timeout: %d\n", timeout);
    printf("domain: %s\n", domain.c_str());
    printf("csv_database: %s\n", csv_database.c_str());
    printf("debug: %d\n", debug);

    udp(dns_port, dns_address, upstream_dns, upstream_port, timeout, domain, csv_database, debug);
}
