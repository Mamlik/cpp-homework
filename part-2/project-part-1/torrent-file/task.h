#pragma once

#include <string>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>

struct TorrentFile {
    std::string announce;
    std::string comment;
    std::vector<std::string> pieceHashes;
    size_t pieceLength;
    size_t length;
    std::string name;
    std::string infoHash;
};


/*
 * Функция парсит .torrent файл и загружает информацию из него в структуру `TorrentFile`. Как устроен .torrent файл, можно
 * почитать в открытых источниках (например http://www.bittorrent.org/beps/bep_0003.html).
 * После парсинга файла нужно также заполнить поле `infoHash`, которое не хранится в файле в явном виде и должно быть
 * вычислено. Алгоритм вычисления этого поля можно найти в открытых источника, как правило, там же,
 * где описание формата .torrent файлов.
 * Данные из файла и infoHash будут использованы для запроса пиров у торрент-трекера. Если структура `TorrentFile`
 * была заполнена правильно, то трекер найдет нужную раздачу в своей базе и ответит списком пиров. Если данные неверны,
 * то сервер ответит ошибкой.
 */


struct BencodeValue;

using ListType = std::vector<BencodeValue>;
using DictType = std::map<std::string, BencodeValue>;

struct BencodeValue {
    std::variant<long long, std::string, ListType, DictType> value;
};

BencodeValue parseBencode(const std::string &content, size_t &pos, bool recordRaw = false, std::string* record = nullptr);

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

BencodeValue parseBencode(const std::string &content, size_t &pos, bool recordRaw, std::string* record) {
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

std::string sha1Raw(const std::string &data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
    return std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
}

TorrentFile LoadTorrentFile(const std::string& filename) {
    TorrentFile torrentFile;
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("File not found");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    size_t pos = 0;
    BencodeValue first = parseBencode(content, pos);
    auto* dict = std::get_if<DictType>(&first.value);
    if (!dict) {
        throw std::runtime_error("Invalid torrent file format");
    }
    torrentFile.announce = std::get<std::string>((*dict)["announce"].value);
    auto comment = dict->find("comment");
    if (comment != dict->end()) {
        torrentFile.comment = std::get<std::string>(comment->second.value);
    }
    
    size_t infoKey = content.find("4:info");
    if (infoKey == std::string::npos) throw std::runtime_error("Invalid torrent file format");
    size_t infoStart = infoKey + 6;
    size_t temp = infoStart;
    std::string infoRaw;
    if (content[temp] != 'd') throw std::runtime_error("Invalid info dict");
    BencodeValue infoVal = parseBencode(content, temp);
    infoRaw = content.substr(infoStart, temp - infoStart);
    
    torrentFile.infoHash = sha1Raw(infoRaw);
    auto* infoDict = std::get_if<DictType>(&infoVal.value);
    if (!infoDict) {
        throw std::runtime_error("Invalid torrent file format");
    }
    
    auto itName = infoDict->find("name");
    if (itName == infoDict->end()) throw std::runtime_error("Missing name");
    torrentFile.name = std::get<std::string>(itName->second.value);
    
    auto itLength = infoDict->find("length");
    if (itLength == infoDict->end()) throw std::runtime_error("Missing length");
    torrentFile.length = std::get<long long>(itLength->second.value);
    
    auto itPieceLength = infoDict->find("piece length");
    if (itPieceLength == infoDict->end()) throw std::runtime_error("Missing piece length");
    torrentFile.pieceLength = std::get<long long>(itPieceLength->second.value);
    
    auto itPieces = infoDict->find("pieces");
    if (itPieces == infoDict->end()) throw std::runtime_error("Missing pieces");
    const std::string &piecesRaw = std::get<std::string>(itPieces->second.value);
    size_t count = piecesRaw.size() / SHA_DIGEST_LENGTH;
    torrentFile.pieceHashes.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        torrentFile.pieceHashes.push_back(piecesRaw.substr(i * SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH));
    }
        
    return torrentFile;
}
