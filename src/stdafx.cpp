#include "../include/stdafx.hpp"
#include <vector>
#include <string>

std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t pos = 0;
    std::size_t found;
    while ((found = input.find(delimiter, pos)) != std::string::npos) {
        result.push_back(input.substr(pos, found - pos));
        pos = found + delimiter.length();
    }
    result.push_back(input.substr(pos));
    return result;
}

