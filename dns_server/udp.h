#include "dns_server_utils.h"

#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <crafter.h>
#include <fstream>
#include <string>
#include <optional>
#include <getopt.h>

#include "../config/DnsMapUser.h"
#include "../config/DnsMapUserSettings.h"
#include "../networking/ip_getter.h"
#include "../networking/crafter_requester.h"

void udp(int dns_port, std::string dns_address, std::string upstream_dns, int upstream_port, int timeout, std::string domain);