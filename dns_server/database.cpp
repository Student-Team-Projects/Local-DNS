#include "database.h"

std::unordered_map<std::string, std::unordered_map<uint32_t, std::vector<addressesResponse>>> addresses;

uint32_t dnsMapKey(uint16_t DNSClass, uint16_t DNSType) {
    return (static_cast<uint32_t>(DNSClass) << 16) | DNSType;
}
uint16_t dnsMapType(uint32_t key) {
    return static_cast<uint16_t>(key & 0x0000FFFF);
}
uint16_t dnsMapClass(uint32_t key) {
    return static_cast<uint16_t>((key & 0xFFFF0000) >> 16);
}


void CSVAppend (std::string filename, Crafter::DNS::DNSAnswer dns, time_t expire, bool local) {
    std::ofstream outfile;
    outfile.open(filename, std::ios::out | std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open or create the file: " << filename << std::endl;
        return;
    }
    outfile << expire << "," << dns.GetName() << "," << dnsMapKey(dns.GetClass(), dns.GetType()) << "," << dns.GetRData() << "," << dns.GetRDataLength() << "," << local << "," << std::endl;
    outfile.close();
}

void CSVAppendLocal (std::string filename, std::string name, addressesResponse dns) {
    std::ofstream outfile;
    outfile.open(filename, std::ios::out | std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open or create the file: " << filename << std::endl;
        return;
    }
    outfile << dns.expired << "," << name << "," << dnsMapKey(1, 1) << "," << dns.RData << "," << dns.RDataLength << "," << dns.local << "," << std::endl;
    outfile.close();
}

void CSVParse (std::string filename) {
    std::ifstream file(filename);
    std::string outfilename = filename + ".tmp";
    std::ofstream outfile;
    outfile.open(outfilename, std::ios::out | std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open or create temporary file '" << outfilename << "'" << std::endl;
        return;
    }
    if (!file.good()) {
        std::cout << "Database file '" << filename << "' does not exist." << std::endl;
        return;
    }

    if (!file.is_open()) {
        std::cerr << "Error: could not open file '" << filename << "'" << std::endl;
        return;
    }

    std::cout << "Reading database from '" << filename << "'" << std::endl;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        uint8_t cells = 0;
        std::string cell;
        addressesResponse row("", 0, 0);
        std::string name;
        uint64_t key;

        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == ',') {
                try {
                    switch(cells) {
                        case 0: { // 'expire' cell
                            time_t timestamp = static_cast<time_t>(std::stoull(cell));
                            if(time(nullptr) < timestamp)
                                row.expired = timestamp;
                            else
                                throw std::runtime_error("Record expired");
                            break;
                        }
                        case 1: // 'name' cell
                            name = cell;
                            break;
                        case 2: // 'key' cell
                            key = static_cast<uint32_t>(std::stoul(cell));
                            break;
                        case 3: // 'RData' cell
                            row.RData = cell;
                            break;
                        case 4: // 'RDataLength' cell
                            row.RDataLength = static_cast<uint16_t>(std::stoul(cell));
                        case 5: // 'local' cell
                            row.local = static_cast<bool>(std::stoul(cell));
                    }
                } catch (std::exception& e) {
                    break;
                }
                cells++;
                cell.clear();
                if (cells == 6) { // All 5 cells read
                    addresses[name][key].push_back(row);
                    outfile << line << std::endl;
                }
            } else {
                cell += c;
            }
        }
    }
    std::cout << "Database read" << std::endl;
    
    file.close();
    outfile.close();

    // Remove the original file
    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error: could not remove the original file '" << filename << "'" << std::endl;
        return;
    }

    // Rename the temporary file to the original file name
    if (std::rename(outfilename.c_str(), filename.c_str()) != 0) {
        std::cerr << "Error: could not rename the temporary file '" << outfilename
                  << "' to '" << filename << "'" << std::endl;
    } else {
        std::cout << "Successfully updated '" << filename << "'" << std::endl;
    }

}