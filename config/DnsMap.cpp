#include <fstream>
#include "DnsMap.h"

#define LINE_PADDING 4

std::string DnsMap::prettifyLine(std::string line, bool insertComma) {
    line = line.substr(1); // remove first {
    line = line.substr(0, line.length() - 1); // remove last }
    line.insert(0, LINE_PADDING, ' '); // add extra padding
    if (insertComma) {
        line += ',';
    }
    return line;
}

void DnsMap::createFile(const std::string &name) {
    if (std::ifstream(name)) { // check if file already exists
        return;
    }
    std::ofstream file(name);
    file << "{\n}";
}

DnsMap::DnsMap(std::string filename) : filename(filename) {
}

void DnsMap::updateMap() {
    updateEntry("", {}, true);
}

void DnsMap::updateEntry(const std::string &key,
                              const std::vector<std::string> &attr, bool updateMap) {
    if (updateMap) {
        this->mac_set.clear();
    }
    std::string copyFileName = filename + "copy";
    createFile(copyFileName);
    std::ofstream copyFile;
    copyFile.open(copyFileName, std::ios_base::trunc);

    std::ifstream configFile(filename);

    std::string line;
    bool entryUpdated = false; // indicate whether key creates new entry

    while (getline(configFile, line)) {
        auto first_character = line.find_first_not_of(' ');
        if (first_character == std::string::npos || line[first_character] == '/' || line[first_character] == '{') {
            copyFile << line << std::endl;
            continue;
        }

        if (line[first_character] == '}') {
            if (entryUpdated) {
                copyFile << line << std::endl;
            } else {
                nlohmann::json j;
                j = {{key, attr}};
                copyFile << prettifyLine(j.dump(), false) << std::endl;
                copyFile << line << std::endl;
            }
            continue;
        }

        auto last_character = line.find_last_not_of(' ');
        bool hasEndComma = false;
        if (line[last_character] == ',') {
            hasEndComma = true;
            line = line.substr(0, last_character);
        }
        nlohmann::json j = nlohmann::json::parse("{" + line + "}");

        auto e = j.items().begin();
        if (updateMap) {
            this->mac_set.insert(static_cast<std::string> (e.value()[0]));
        }


        if (e.key() == key) {

            j[e.key()] = attr;
            entryUpdated = true;
        }

        copyFile << prettifyLine(j.dump(), hasEndComma || !entryUpdated) << std::endl;
    }
    if (updateMap) {
        remove(copyFileName.c_str());
        return;
    }

    remove(filename.c_str());
    rename(copyFileName.c_str(), filename.c_str());
}


std::vector<std::string> DnsMap::getEntry(const std::string &key) {
    std::ifstream inputFile(filename);
    nlohmann::json j = nlohmann::json::parse(inputFile, nullptr, true, true);
    if (j.contains(key)) {
        return j[key];
    }

    return {};
}

std::unordered_set<std::string> DnsMap::entries() {
    updateMap();
    return this->mac_set;
}
