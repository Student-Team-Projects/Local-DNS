#include <thread>
#include <iostream>

#include "crafter_requester.h"

CrafterRequester::CrafterRequester(std::string iface) : Requester(), iface(iface) {
    myIP = Crafter::GetMyIP(iface);
    if (myIP.empty()) {
        std::cerr << "Local DNS error: invalid interface" << std::endl;
        exit(1);
    }
    myMAC = Crafter::GetMyMAC(iface);
    if (myMAC.empty()) {
        std::cerr << "Local DNS error: invalid interface" << std::endl;
        exit(1);
    }
     
    ethernetHeaderTemplate.SetSourceMAC(myMAC);
    arpHeaderTemplate.SetOperation(Crafter::ARP::Request);
    arpHeaderTemplate.SetSenderIP(myIP);
    arpHeaderTemplate.SetSenderMAC(myMAC);

    listen_for_requests();
}

void CrafterRequester::listen_for_requests() {
    std::thread([this]() {
            while (true) {
                auto request = this->requests.pop();
                std::string ipMask = request.first;
                std::string mac = request.second;
                this->ethernetHeaderTemplate.SetDestinationMAC(mac);
                std::vector<std::string> net = Crafter::GetIPs(ipMask);
                std::vector<Crafter::Packet*> packets;
                for (auto ipAddr = net.begin(); ipAddr != net.end(); ipAddr++) {
                    this->arpHeaderTemplate.SetTargetIP(*ipAddr);
                    Crafter::Packet *packet = new Crafter::Packet;
                    packet->PushLayer(this->ethernetHeaderTemplate);
                    packet->PushLayer(this->arpHeaderTemplate);
                    packets.push_back(packet);
                }
                std::vector<Crafter::Packet*> replies_packets(packets.size());
                Crafter::SendRecv(packets.begin(), packets.end(), replies_packets.begin(), iface, 0.1, 2, 48);
                
                std::vector<Crafter::Packet*>::iterator it_pck;
                for(it_pck = replies_packets.begin() ; it_pck < replies_packets.end() ; it_pck++) {
                    Crafter::Packet* reply_packet = (*it_pck);
                    if(reply_packet) {
                        Crafter::ARP* arp_layer = reply_packet->GetLayer<Crafter::ARP>();
                        std::string mac = arp_layer->GetSenderMAC();
                        std::string IP = arp_layer->GetSenderIP();
                        std::lock_guard<std::mutex> guard(this->map_lock);
                        auto promise = this->map.find(mac);
                        if (promise != this->map.end()){
                            promise->second->set_value(IP);
                            this->map.erase(promise);
                        }
                    }

                }

                Crafter::ClearContainer(packets);
                Crafter::ClearContainer(replies_packets);
        }
    }).detach();
}

void CrafterRequester::request(std::string ip_mask, std::string mac_requested) {
	requests.push(make_pair(ip_mask, mac_requested));
}
