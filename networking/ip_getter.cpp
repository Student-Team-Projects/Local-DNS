#include "ip_getter.h"
#include "../lib/time_utils.h"
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <thread>
#include <utility>
#include <vector>

IPGetter::
    IPGetter(CrafterRequester* requester, DnsMapCache* dnsMapCache, std::string local_network_ip_mask, int timeout)
    : requester(requester)
    , dnsMapCache(dnsMapCache)
    , map(&requester->map)
    , map_lock(requester->map_lock)
    , local_network_ip_mask(local_network_ip_mask)
    , timeout(timeout) {
}

std::string IPGetter::wait_for_promise(std::promise<std::string>& promise) {
    auto future = promise.get_future();
    auto status = future.wait_for(std::chrono::milliseconds(timeout));
    if(status == std::future_status::timeout) {
        return "";
    } else {
        return future.get();
    }
}

std::string IPGetter::get_ip(std::string mac, int cacheTimeout) {
    std::vector<std::string> cacheAttributes = dnsMapCache->getIpAttributes(mac);
    if(!cacheAttributes.empty() && TimeUtils::valid(cacheAttributes[1], cacheTimeout)) {
        return cacheAttributes[0];
    }

    std::shared_ptr<std::promise<std::string>> result = std::make_shared<std::promise<std::string>>();
    std::pair<std::map<std::string, std::shared_ptr<std::promise<std::string>>>::iterator, bool> entry;
    {
        std::lock_guard<std::mutex> guard(map_lock);
        entry = map->emplace(mac, result);
    }
    requester->request(local_network_ip_mask, mac);
    std::string resultIP = wait_for_promise(*result);
    return resultIP;
}
