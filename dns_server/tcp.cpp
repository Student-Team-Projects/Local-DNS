#include "tcp.h"

void tcp(int dns_port, std::string dns_address, std::string upstream_dns, int upstream_port, int timeout, std::string domain){
    DnsMapUser dnsMapUser;
    DnsMapUserSettings dnsMapUserSettings;
    DnsMapCache dnsMapCache;
    std::string dnsRedirect = getDnsServerRedirect(dns_address, upstream_dns);


    CrafterRequester requester(dnsMapUserSettings.get_setting("iface"));

    dnsMapCache.synchronizeCacheWithUserConfig(dnsMapUser);
    IPGetter ipgetter(&requester, &dnsMapCache, dnsMapUserSettings.get_setting("ip_mask"), timeout);

    int cacheTimeout = getCacheTimeout(dnsMapUserSettings);

    struct sockaddr_in server =
            {
                    .sin_family = AF_INET,
                    .sin_port = htons(dns_port)
            };

    inet_pton(AF_INET, dns_address.c_str(), &server.sin_addr);
    const int socket_ = socket(AF_INET, SOCK_STREAM, 0);

    socklen_t len = sizeof(server);
    bind(socket_, (struct sockaddr *) &server, len);
    listen(socket_, 10);
    while (true) {
        struct sockaddr_in client = {};


        long n;
        int fd;
        unsigned char buffer[BUFFER_SIZE] = {};
        memset(buffer, 0, sizeof(buffer));
        fd = accept(socket_, (struct sockaddr *) &client, &len);
        n = read(fd, buffer, sizeof(buffer));
        std::cout << buffer << std::endl;
        Crafter::RawLayer raw(reinterpret_cast<const unsigned char *>(buffer + 2), n - 2);
        Crafter::DNS dns;
        dns.FromRaw(raw);
        Crafter::DNS::DNSQuery dnsQuery(dns.Queries[0]);
        std::cout << dnsQuery.GetName() << std::endl;

        if ((dnsQuery.GetType() == Crafter::DNS::TypeA || dnsQuery.GetType() == Crafter::DNS::TypeANY) &&
            dnsQuery.GetName().ends_with("." + domain)) {
            std::string mac = dnsMapUser.getMacFromDnsName(dnsQuery.GetName());
            if (!mac.empty()) {
                std::string ip_addr = ipgetter.get_ip(mac, cacheTimeout);
                std::cout << ip_addr << std::endl;
                if (!ip_addr.empty()) {
                    pid_t pid = fork();
                    if(pid == 0) {
                        buffer[4] = 0x85; // flags: qr aa rd ra - byte 0
                        buffer[5] = 0x80; // flags: qr aa rd ra - byte 1
                        buffer[9] = 0x01; // # of answers
                        buffer[n] = 0xc0; // beginning mark
                        buffer[n + 1] = 0x0c; // offset to domain in query
                        buffer[n + 2] = 0x00; // Crafter::DNS::TypeA - byte 0
                        buffer[n + 3] = 0x01; // Crafter::DNS::TypeA - byte 1
                        buffer[n + 4] = 0x00; // Crafter::DNS::ClassIN - byte 0
                        buffer[n + 5] = 0x01; // Crafter::DNS::ClassIN - byte 1
                        buffer[n + 6] = 0x00; // ttl=60 - byte 0
                        buffer[n + 7] = 0x00; // ttl=60 - byte 1
                        buffer[n + 8] = 0x00; // ttl=60 - byte 2
                        buffer[n + 9] = 0x3c; // ttl=60 - byte 3
                        buffer[n + 10] = 0x00; // rdlength=4 - byte 0
                        buffer[n + 11] = 0x04; // rdlength=4 - byte 1
                        sscanf(
                            ip_addr.c_str(),
                            "%hhu.%hhu.%hhu.%hhu",
                            &buffer[n + 12],
                            &buffer[n + 13],
                            &buffer[n + 14],
                            &buffer[n + 15]
                        );

                        n += 16;
                        buffer[1] += 16; // FIXME
                        write(fd, buffer, n);
                        exit(0);
                    }
                    continue;
                }
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            std::cout << "using default dns\n";
            struct sockaddr_in dns_server =
                    {
                            .sin_family = AF_INET,
                            .sin_port = htons(upstream_port)
                    };


            inet_pton(AF_INET, dnsRedirect.c_str(), &dns_server.sin_addr);
            const int dns_socket_ = socket(AF_INET, SOCK_STREAM, 0);
            socklen_t dns_len = sizeof(dns_server);
            connect(dns_socket_, (struct sockaddr *) &dns_server, dns_len);
            send(dns_socket_, buffer, n, 0);

            memset(buffer, 0, sizeof(buffer));
            n = read(dns_socket_, buffer, sizeof(buffer));
            shutdown(dns_socket_, SHUT_RDWR);

            write(fd, buffer, n);
            exit(0);
        }
    }
}
