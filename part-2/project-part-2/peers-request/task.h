#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <cpr/cpr.h>


struct BencodeValue {
    std::variant<long long, std::string, std::vector<BencodeValue>, std::map<std::string, BencodeValue>> value;
};
using ListType = std::vector<BencodeValue>;
using DictType = std::map<std::string, BencodeValue>;

BencodeValue parseBencode(const std::string &content, size_t &pos);


class TorrentTracker {
public:
    /*
     * url - адрес трекера, берется из поля announce в .torrent-файле
     */
    TorrentTracker(const std::string& url) : url_(url) {}

    /*
     * Получить список пиров у трекера и сохранить его для дальнейшей работы.
     * Запрос пиров происходит посредством HTTP GET запроса, данные передаются в формате bencode.
     * Такой же формат использовался в .torrent файле.
     * Подсказка: посмотрите, что было написано в main.cpp в домашнем задании torrent-file
     *
     * tf: структура с разобранными данными из .torrent файла из предыдущего домашнего задания.
     * peerId: id, под которым представляется наш клиент.
     * port: порт, на котором наш клиент будет слушать входящие соединения (пока что мы не слушаем и на этот порт никто
     *  не сможет подключиться).
     */
    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port) {
        peers_.clear();
        cpr::Response res = cpr::Get(
            cpr::Url{url_},
            cpr::Parameters {
                    {"info_hash", tf.infoHash},
                    {"peer_id", peerId},
                    {"port", std::to_string(port)},
                    {"uploaded", std::to_string(0)},
                    {"downloaded", std::to_string(0)},
                    {"left", std::to_string(tf.length)},
                    {"compact", std::to_string(1)}
            },
            cpr::Timeout{20000}
        );
        if (res.status_code == 200) {
            if (res.text.find("failure reason") == std::string::npos) {
                size_t pos = 0;
                BencodeValue first = parseBencode(res.text, pos);
                auto* dict = std::get_if<DictType>(&first.value);
                if (!dict) throw std::runtime_error("Invalid tracker response format");
                auto itPeers = dict->find("peers");
                if (itPeers == dict->end()) throw std::runtime_error("Missing peers in tracker response");
                const std::string& peersRaw = std::get<std::string>(itPeers->second.value);
                for (size_t i = 0; i < peersRaw.size(); i += 6) {
                    Peer peer;
                    std::string block = peersRaw.substr(i, 6);

                    peer.ip = std::to_string((unsigned char)block[0]) + "." +
                              std::to_string((unsigned char)block[1]) + "." +
                              std::to_string((unsigned char)block[2]) + "." +
                              std::to_string((unsigned char)block[3]);
                    uint16_t port = ((unsigned char)(block[4]) << 8) | (unsigned char)(block[5]);
                    peer.port = port;
                    peers_.push_back(peer);
                }

            } else {
                throw std::runtime_error("Server responded with failure reason: " + res.text);
            }
        } else {
            throw std::runtime_error("Failed to get peers from tracker: " + std::to_string(res.status_code) + " " + res.status_line);
        }
    }

    /*
     * Отдает полученный ранее список пиров
     */
    const std::vector<Peer>& GetPeers() const {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};

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

////