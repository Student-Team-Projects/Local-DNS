#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <crafter.h>
#include <fstream>
#include <string>
#include <getopt.h>

#include "../config/DnsMapUser.h"
#include "../config/DnsMapUserSettings.h"
#include "../networking/ip_getter.h"
#include "../networking/crafter_requester.h"

#define BUFFER_SIZE 1024
std::string getDnsServerRedirect(std::string, std::string);

void udp(int dns_port, std::string dns_address, std::string upstream_dns, int upstream_port, int timeout, std::string domain){

    Crafter::InitCrafter();
    DnsMapUser dnsMapUser;
    DnsMapUserSettings dnsMapUserSettings;
    std::string dnsRedirect = getDnsServerRedirect(dns_address, upstream_dns);


    CrafterRequester requester(dnsMapUserSettings.get_setting("iface"));
    IPGetter ipgetter(&requester, dnsMapUser, dnsMapUserSettings.get_setting("ip_mask"), timeout);


    struct sockaddr_in server =
            {
                    .sin_family = AF_INET,
                    .sin_port = htons(dns_port)
            };

    inet_pton(AF_INET, dns_address.c_str(), &server.sin_addr);
    const int socket_ = socket(AF_INET, SOCK_DGRAM, 0);

    socklen_t len = sizeof(server);
    bind(socket_, (struct sockaddr *) &server, len);

    while (true) {
        struct sockaddr_in client = {};


        long n;
        unsigned char buffer[BUFFER_SIZE] = {};
        memset(buffer, 0, sizeof(buffer));
        n = recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client, &len);
        Crafter::RawLayer raw(reinterpret_cast<const unsigned char *>(buffer), n);
        Crafter::DNS dns;
        dns.FromRaw(raw);
        Crafter::DNS::DNSQuery dnsQuery(dns.Queries[0]);
        std::cout << dnsQuery.GetName() << std::endl;

        if ((dnsQuery.GetType() == Crafter::DNS::TypeA || dnsQuery.GetType() == Crafter::DNS::TypeANY) &&
            dnsQuery.GetName().ends_with("." + domain)) {
            std::string mac = dnsMapUser.getMacFromDnsName(dnsQuery.GetName());
            std::cout << mac << std::endl;
            if (!mac.empty()) {
                std::string ip_addr = ipgetter.get_ip(mac, dnsMapUserSettings.get_setting("cache_timeout"));
                std::cout << ip_addr << std::endl;
                pid_t pid = fork();
                if (pid == 0) {
                    buffer[2] = 0x85;                               //flags: qr aa rd ra - byte 0
                    buffer[3] = 0x80;                               //flags: qr aa rd ra - byte 1
                    buffer[7] = 0x01;                               //# of answers
                    buffer[n] = 0xc0;                               //beginning mark
                    buffer[n + 1] = 0x0c;                             //offset to domain in query
                    buffer[n + 2] = 0x00;                             //Crafter::DNS::TypeA - byte 0
                    buffer[n + 3] = 0x01;                             //Crafter::DNS::TypeA - byte 1
                    buffer[n + 4] = 0x00;                             //Crafter::DNS::ClassIN - byte 0
                    buffer[n + 5] = 0x01;                             //Crafter::DNS::ClassIN - byte 1
                    buffer[n + 6] = 0x00;                             //ttl=60 - byte 0
                    buffer[n + 7] = 0x00;                             //ttl=60 - byte 1
                    buffer[n + 8] = 0x00;                             //ttl=60 - byte 2
                    buffer[n + 9] = 0x3c;                             //ttl=60 - byte 3
                    buffer[n + 10] = 0x00;                            //rdlength=4 - byte 0
                    buffer[n + 11] = 0x04;                            //rdlength=4 - byte 1
                    sscanf(ip_addr.c_str(), "%hhu.%hhu.%hhu.%hhu", &buffer[n + 12], &buffer[n + 13],
                           &buffer[n + 14],
                           &buffer[n + 15]);

                    n += 16;

                    sendto(socket_, buffer, n, 0, (struct sockaddr *) &client, len);
                    exit(0);
                }
                continue;
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            struct sockaddr_in dns_server =
                    {
                            .sin_family = AF_INET,
                            .sin_port = htons(upstream_port)
                    };


            inet_pton(AF_INET, dnsRedirect.c_str(), &dns_server.sin_addr);
            const int dns_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
            socklen_t dns_len = sizeof(dns_server);

            sendto(dns_socket_, buffer, n, 0, (struct sockaddr *) &dns_server, dns_len);

            struct sockaddr_in from = {};
            memset(buffer, 0, sizeof(buffer));
            n = recvfrom(dns_socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &from, &dns_len);

            shutdown(dns_socket_, SHUT_RDWR);

            sendto(socket_, buffer, n, 0, (struct sockaddr *) &client, len);
            exit(0);
        }
    }
}