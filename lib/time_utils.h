#pragma once
#include <string>

class TimeUtils {
    public:
        static bool valid(std::string timestamp, int termOfValidityDays);
        static std::string timeNow();
};
