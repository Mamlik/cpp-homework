#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>


struct BencodeValue {
    std::variant<long long, std::string, std::vector<BencodeValue>, std::map<std::string, BencodeValue>> value;
};
using ListType = std::vector<BencodeValue>;
using DictType = std::map<std::string, BencodeValue>;

BencodeValue parseBencode(const std::string &content, size_t &pos);