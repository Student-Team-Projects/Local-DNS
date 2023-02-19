#ifndef CRAFTER_REQUESTER_H

#define CRAFTER_REQUESTER_H

#include "requester.h"
#include "../lib/blocking_queue.h"
#include "crafter.h"
#include <future>
#include <map>
#include <string>

class IPGetter;

class CrafterRequester: public Requester
{  
	private:
	  std::map<std::string, std::promise<std::string>*> map;
          std::mutex map_lock;
          
          std::string iface;

          std::string myIP;
          std::string myMAC;

          Crafter::Ethernet ethernetHeaderTemplate;
          Crafter::ARP arpHeaderTemplate;

          BlockingQueue<std::pair<std::string, std::string>> requests;

          void listen_for_requests();
    
	public:
	  friend IPGetter;
	  CrafterRequester(std::string iface);
          virtual void request(std::string ip_mask, std::string mac_requested); 
};

#endif
        
