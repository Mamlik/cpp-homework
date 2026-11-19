#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>


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