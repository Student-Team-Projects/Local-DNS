#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

#include <cstdint>

uint32_t constexpr MIN_HOST_BITS = 2;
uint32_t constexpr MAX_HOST_BITS = 8;

std::set<std::string> getIPs(std::string const& baseIP, std::string const& maskDotted);
std::optional<uint32_t> dottedToBinary(std::string const& address);
std::optional<uint32_t> hostBitsOf(std::string const& mask);
std::optional<uint32_t> hostBitsOf(uint32_t mask);
std::optional<std::string> getShortestValidMask(std::vector<std::string> const& masks);
