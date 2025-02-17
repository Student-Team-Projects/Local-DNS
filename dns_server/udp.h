#pragma once

#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <crafter.h>
#include <fstream>
#include <string>

#include "settings.h"
#include "database.h"

#define BUFFER_SIZE 1024

void udp(
    int dns_port,
    std::string dns_address,
    std::string upstream_dns,
    int upstream_port,
    int timeout,
    std::string domain,
    std::string csv,
    bool debug
);
