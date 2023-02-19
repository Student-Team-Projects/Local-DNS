#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include "../lib/nlohmann/json.hpp"


class DnsMap {
private:
    std::string filename;
    std::unordered_set<std::string> mac_set;

    static std::string prettifyLine(std::string line, bool insertComma);

public:
    DnsMap(std::string filename);

    static void createFile(const std::string &filename);

    std::unordered_set<std::string> entries();

    void updateMap();

    void
    updateEntry(const std::string &key, const std::vector<std::string> &attr, bool updateMap = false);

    std::vector<std::string> getEntry(const std::string &key);
};
