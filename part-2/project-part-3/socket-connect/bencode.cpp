#include "bencode.h"



BencodeValue parseInteger(const std::string &content, size_t &pos) {
    if (content[pos] != 'i') {
        throw std::runtime_error("Invalid integer format(not integer)");
    }
    pos++;
    size_t end_pos = content.find('e', pos);
    if (end_pos == std::string::npos) {
        throw std::runtime_error("Invalid integer format(didnt find end)");
    }
    std::string int_str = content.substr(pos, end_pos - pos);
    pos = end_pos + 1;
    BencodeValue result;
    result.value = std::stoll(int_str);
    return result;
}

BencodeValue parseString(const std::string &content, size_t &pos) {
    size_t colon_pos = content.find(':', pos);
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Invalid string format(didnt find colon)");
    }
    size_t length = std::stoull(content.substr(pos, colon_pos - pos));
    pos = colon_pos + 1;
    std::string str = content.substr(pos, length);
    pos += length;
    BencodeValue result;
    result.value = str;
    return result;
}

BencodeValue parseList(const std::string &content, size_t &pos) {
    if (content[pos] != 'l') {
        throw std::runtime_error("Invalid list format(not list)");
    }
    pos++;
    ListType list;
    while (content[pos] != 'e') {
        list.push_back(parseBencode(content, pos));
    }
    pos++;
    BencodeValue result;
    result.value = list;
    return result;
}

BencodeValue parseDict(const std::string &content, size_t &pos) {
    if (content[pos] != 'd') throw std::runtime_error("Invalid dict format");
    ++pos;
    DictType dict;
    while (content[pos] != 'e') {
        BencodeValue keyVal = parseString(content, pos);
        const std::string &key = std::get<std::string>(keyVal.value);
        BencodeValue val = parseBencode(content, pos);
        dict.emplace(key, std::move(val));
    }
    ++pos;
    return BencodeValue{dict};
}

BencodeValue parseBencode(const std::string &content, size_t &pos) {
    if (pos >= content.size()) {
        throw std::runtime_error("Invalid bencode format(out of range)");
    }
    if (content[pos] == 'l') {
        return parseList(content, pos);
    } else if (content[pos] == 'i') {
        return parseInteger(content, pos);
    } else if (content[pos] == 'd') {
        return parseDict(content, pos);
    } else if (isdigit(content[pos])) {
        return parseString(content, pos);
    } else {
        throw std::runtime_error("Invalid bencode format(unknown type)");
    }
}