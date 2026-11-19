#pragma once

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

#include "torrent_file.h"
#include "peer.h"
#include "bencode.h"





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
        const std::vector<Peer>& GetPeers() const;
    
    private:
        std::string url_;
        std::vector<Peer> peers_;
    };

