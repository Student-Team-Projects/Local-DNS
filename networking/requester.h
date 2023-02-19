//Abstract class that provides a single method request()
//Isolates the library-specific part from the abstraction of ip_getter()
#pragma once
#include <string> 

class Requester
{
    protected:
        Requester() {}
    public:
        virtual void request(std::string ip_mask, std::string mac_requested) = 0;
        /*
        * not sure if using strings is the best here, 
        * intention is ip_getter doesn't have to worry about using the same data structures the library is using
        */
};
