#pragma once

#include <fstream>
#include "bencode.h"
#include <variant>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <cpr/cpr.h>
#include <string>
#include <vector>
#include <openssl/sha.h>

struct TorrentFile {
    std::string announce;
    std::string comment;
    std::vector<std::string> pieceHashes;
    size_t pieceLength;
    size_t length;
    std::string name;
    std::string infoHash;
};


std::string sha1Raw(const std::string &data);


TorrentFile LoadTorrentFile(const std::string& filename);