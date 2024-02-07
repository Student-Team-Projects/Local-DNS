#ifndef CRAFTER_REQUESTER_H

#define CRAFTER_REQUESTER_H

#include "requester.h"
#include "../lib/blocking_queue.h"
#include "../config/DnsMapCache.h"
#include "../lib/time_utils.h"
#include "crafter.h"
#include <future>
#include <map>
#include <string>
#include <memory>

class IPGetter;

class CrafterRequester: public Requester
{  
	private:
	  std::map<std::string, std::shared_ptr<std::promise<std::string>>> map;
          std::mutex map_lock;
          
          std::string iface;

          std::string myIP;

          Crafter::Ethernet ethernetHeaderTemplate;
          Crafter::ARP arpHeaderTemplate;

          BlockingQueue<std::pair<std::string, std::string>> requests;

          DnsMapCache *dnsMapCache;  
          void listen_for_requests();
    
	public:
	  friend IPGetter;
	  CrafterRequester(std::string iface, DnsMapCache *dnsMapCache);
          virtual void request(std::string ip_mask, std::string mac_requested); 
};

#endif
        
