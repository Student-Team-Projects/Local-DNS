// IPGetter isolates the entire local-dns algorithm
#pragma once

#include <string>
#include <map>
#include <utility>
#include <future>

#include "crafter_requester.h"
#include "../config/DnsMapCache.h"


class IPGetter
{
private:
    std::string local_network_ip_mask; //Something like "192.168.0.*"
    CrafterRequester *requester;
 
    std::map<std::string, std::promise<std::string>*>* map;
    std::mutex& map_lock;

    int timeout; //in miliseconds

    DnsMapCache *dnsMapCache;

    std::string wait_for_promise(std::promise<std::string>& promise);
public:
    std::string get_ip(std::string mac, int cacheTimeout); //This is the API the rest of the program has to call in order to invoke local-dns for a certain mac address
    
    IPGetter(CrafterRequester *requester, DnsMapCache *dnsMapCache, std::string local_network_ip_mask, int timeout);
};

