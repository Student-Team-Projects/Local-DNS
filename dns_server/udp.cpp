#include "udp.h"

void udp(
    int dns_port,
    std::string dns_address,
    std::string upstream_dns,
    int upstream_port,
    int timeout,
    std::string domain,
    std::string csv,
    bool debug
) {
    CSVFile = csv;
    bool caching = static_cast<bool>(CSVFile.length());

    Crafter::InitCrafter();

    std::string dnsRedirect = getDnsServerRedirect(dns_address, upstream_dns);


    if(caching)
        CSVParse(CSVFile); // fill addresses map with saved database content
    config();
    printSettings();



    struct sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_port = htons(dns_port);
    inet_pton(AF_INET, dns_address.c_str(), &server.sin_addr);

    int const socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(server);
    if (bind(socket_, (struct sockaddr*)&server, len) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;
        exit(EXIT_FAILURE);
    }



    while (true) {
        if(debug)
            std::cout << std::endl;
        struct sockaddr_in client = {};
        unsigned char buffer[BUFFER_SIZE] = {};
        memset(buffer, 0, sizeof(buffer));

        long n = recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &len);
        if (n <= 0) {
            std::cerr << "recvfrom failed or no data received." << std::endl;
            continue;
        }

        Crafter::RawLayer raw(reinterpret_cast<unsigned char const*>(buffer), n);
        Crafter::DNS dns;
        dns.FromRaw(raw);

        if (dns.Queries.empty()) {
            // No valid query found
            continue;
        }
        Crafter::DNS::DNSQuery dnsQuery(dns.Queries[0]);
        std::string DNSName = dnsQuery.GetName();
        uint16_t DNSType = dnsQuery.GetType();
        uint16_t DNSClass = dnsQuery.GetClass();
        uint32_t DNSMapKey = dnsMapKey(DNSClass, DNSType);
        std::cout << "Queried: " << DNSName << std::endl;
        if(debug)
            std::cout << "Class: " << DNSClass << std::endl << "Type: " << DNSType << std::endl;

        if (addresses.contains(DNSName) && addresses[DNSName].contains(DNSMapKey)) {
            if(debug)
                std::cout << "This domain, class and type exist in local database" << std::endl;
            bool areTTLsValid = true;
            Crafter::DNS response;
            for (int i = 0; i < addresses[DNSName][DNSMapKey].size(); i++) {
                int64_t TTL = addresses[DNSName][DNSMapKey][i].expired - time(nullptr);
                if(TTL > 0) {
                    Crafter::DNS::DNSAnswer dnsAnswer(DNSName, addresses[DNSName][DNSMapKey][i].RData);
                    dnsAnswer.SetType(DNSType);
                    dnsAnswer.SetClass(DNSClass);
                    dnsAnswer.SetTTL(TTL);
                    response.Answers.push_back(dnsAnswer);
                }
                else {
                    if(debug)
                        std::cout << "Some records have expired" << std::endl;
                    if(addresses[DNSName][DNSMapKey][i].local) {
                        std::cout << "Expired domain in local network" << std::endl;
                        updateLocalDNS();
                        i--;
                    }
                    else {
                        areTTLsValid = false;
                        break;
                    }
                }
            }
            if (areTTLsValid) {
                response.SetIdentification(dns.GetIdentification());  // Must match the client's request ID
                response.SetQRFlag(1);    // This is a response
                response.SetAAFlag(1);    // Set if you are authoritative, else 0
                response.SetRDFlag(0);    // Depends on if recursion was requested/provided
                response.SetRAFlag(0);    // Usually set to 0 if no recursion available, 1 if yes
                response.SetRCode(Crafter::DNS::RCodeNoError); // No error

                response.Queries.push_back(dnsQuery);

                response.SetTotalQuestions(1);
                response.SetTotalAnswer(response.Answers.size());
                response.SetTotalAuthority(0);
                response.SetTotalAdditional(0);

                Crafter::Packet responsePacket;
                responsePacket.PushLayer(response);

                const unsigned char* responseData = reinterpret_cast<const unsigned char*>(responsePacket.GetRawPtr());
                size_t responseSize = responsePacket.GetSize();
                
                size_t bytesSent = sendto(socket_, responseData, responseSize, 0, (struct sockaddr*)&client, len);
                if(debug)
                    std::cout << "Response sent using data from local database" << std::endl;
                continue;
            }
        }

        if (true) {
            if(debug)
                std::cout << "Forwarding query upstream" << std::endl;
            struct sockaddr_in dns_server = {};
            dns_server.sin_family = AF_INET;
            dns_server.sin_port = htons(upstream_port);
            inet_pton(AF_INET, dnsRedirect.c_str(), &dns_server.sin_addr);

            int dns_socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (dns_socket_ < 0) {
                std::cerr << "Failed to create upstream socket." << std::endl;
                continue;
            }

            socklen_t dns_len = sizeof(dns_server);
            size_t bytesSentUpstream = sendto(dns_socket_, buffer, n, 0, (struct sockaddr*)&dns_server, dns_len);
            if (bytesSentUpstream <= 0) {
                std::cerr << "Failed to send request to the upstream server." << std::endl;
                continue;
            }

            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            setsockopt(dns_socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            // Receive response from upstream
            unsigned char upstream_buffer[BUFFER_SIZE] = {};
            memset(upstream_buffer, 0, sizeof(upstream_buffer));
            long upstream_n = recvfrom(dns_socket_, upstream_buffer, sizeof(upstream_buffer), 0, (struct sockaddr*)&dns_server, &dns_len);

            shutdown(dns_socket_, SHUT_RDWR);
            close(dns_socket_);

            if (upstream_n > 0) {
                if(debug)
                    std::cout << "Response received from upstream server" << std::endl;
                // Send upstream response back to client
                size_t bytesSent = sendto(socket_, upstream_buffer, upstream_n, 0, (struct sockaddr*)&client, len);
                if (bytesSent <= 0) {
                    std::cerr << "Failed to send answer to client." << std::endl;
                    continue;
                }
                if(debug)
                    std::cout << "Response forwarded to client" << std::endl;

                Crafter::RawLayer raw_layer(upstream_buffer, upstream_n);
                Crafter::DNS dns_response;
                dns_response.FromRaw(raw_layer);
                size_t answerCount = dns_response.GetTotalAnswer();
                if(debug)
                    std::cout << "Received " << answerCount << " answers" << std::endl;
                uint32_t ansKey = 0;
                std::string ansName = "";
                if (answerCount > 0) {
                    ansKey = dnsMapKey(dns_response.Answers[0].GetClass(), dns_response.Answers[0].GetType());
                    ansName = dns_response.Answers[0].GetName();
                    if (addresses.contains(ansName) && addresses[ansName].contains(ansKey))
                        addresses[ansName][ansKey].clear();
                }
                
                for (size_t i = 0; i < answerCount; ++i) {
                    const Crafter::DNS::DNSAnswer& ans = dns_response.Answers[i];
                    time_t expire = time(nullptr) + ans.GetTTL();
                    addressesResponse adr(ans.GetRData(), ans.GetRDataLength(), expire);
                    addresses[ansName][ansKey].push_back(adr);
                    if(caching)
                        CSVAppend(CSVFile, ans, expire, false);
                }
            }
            else {
                std::cerr << "Failed to receive response from upstream server" << std::endl;
                continue;
            }
        }
    }

    close(socket_);
    Crafter::CleanCrafter();
}

