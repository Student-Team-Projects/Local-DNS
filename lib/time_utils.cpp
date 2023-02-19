#include "time_utils.h"
#include <chrono>
#include <iomanip>
#include <sstream>

#define TIME_FORMAT "UTC: %Y-%m-%d %H:%M:%S"

bool TimeUtils::valid(std::string timestamp, int termOfValidityDays) {
    std::stringstream ss(timestamp);
    std::tm tm;
    ss >> std::get_time(&tm, TIME_FORMAT);
    auto timepointIn = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto timepointNow= std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::days>(timepointNow - timepointIn);
    return diff.count() < termOfValidityDays;
}

std::string TimeUtils::timeNow() {
    std::time_t ttnow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm = *std::gmtime(&ttnow);
    std::stringstream ss;
    ss << std::put_time(&tm, TIME_FORMAT);
    return ss.str();
}
