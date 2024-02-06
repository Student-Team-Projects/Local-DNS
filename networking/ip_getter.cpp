#include "ip_getter.h"
#include "../lib/time_utils.h"
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <thread>
#include <utility>
#include <vector>

IPGetter::IPGetter(CrafterRequester *requester, DnsMapUser& dnsMapUser, std::string local_network_ip_mask, int timeout)
    : requester(requester)
    , map(&requester->map)
    , map_lock(requester->map_lock)
    , local_network_ip_mask(local_network_ip_mask)
    , timeout(timeout) {
     dnsMapCache.synchronizeCacheWithUserConfig(dnsMapUser);
}

std::string IPGetter::wait_for_promise(std::promise<std::string> &promise) {
   auto future = promise.get_future();
   auto status = future.wait_for(std::chrono::milliseconds(timeout));
   if (status == std::future_status::timeout) {
      return "";
   } else {
      return future.get();
   }
}

std::string IPGetter::get_ip(std::string mac, int cacheTimeout) {
   //std::cout << "Request: MAC=" << mac << std::endl;

   std::vector<std::string> cacheAttributes = dnsMapCache.getIpAttributes(mac);
   if (!cacheAttributes.empty() && TimeUtils::valid(cacheAttributes[1], cacheTimeout)) {
       //std::cout << "Using cache" << std::endl;
       return cacheAttributes[0];
   }

   std::promise<std::string> result;
   std::pair<std::map<std::string, std::promise<std::string> *>::iterator, bool> entry;
   {
      std::lock_guard<std::mutex> guard(map_lock);
      entry = map->emplace(mac, &result);
   }
   requester->request(local_network_ip_mask, mac);
   std::string resultIP = wait_for_promise(result);
   if (resultIP.empty()) {
       //std::cerr << "error: empty IP" << std::endl;
       //exit(1);
   } else {
       cacheAttributes.push_back(resultIP);
       cacheAttributes.push_back(TimeUtils::timeNow());
       dnsMapCache.updateEntry(mac, cacheAttributes);
   }
   return resultIP;
}
