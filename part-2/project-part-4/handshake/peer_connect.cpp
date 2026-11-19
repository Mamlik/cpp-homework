#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>

using namespace std::chrono_literals;

PeerConnect::PeerConnect(const Peer& peer, const TorrentFile& tf, std::string selfPeerId)
  : tf_(tf),
    socket_(peer.ip, peer.port, 3s, 3s),
    selfPeerId_(std::move(selfPeerId)),
    terminated_(false),
    choked_(true)
{}

void PeerConnect::Run() {
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            MainLoop();
        } else {
            std::cerr << "Cannot establish connection to peer" << std::endl;
            Terminate();
        }
    }
}

void PeerConnect::PerformHandshake() {
    socket_.EstablishConnection();
    std::string handshake;
    handshake.reserve(68);
    handshake += (static_cast<char>(19));
    handshake += "BitTorrent protocol";
    handshake.append(8, '\0');
    handshake += tf_.infoHash;
    if (selfPeerId_.size() != 20) {
        throw std::runtime_error("PerformHandshake: invalid selfPeerId length");
    }
    handshake += selfPeerId_;
    socket_.SendData(handshake);


    std::string response = socket_.ReceiveData(68);
    if (response.size() != 68) {
        throw std::runtime_error("PerformHandshake: invalid response length");
    }
    uint8_t pstrlen = static_cast<uint8_t>(response[0]);
    if (pstrlen != 19 || response.substr(1, 19) != "BitTorrent protocol") {
        throw std::runtime_error("PerformHandshake: invalid protocol string");
    }
    std::string peerInfoHash = response.substr(1 + 19 + 8, 20);
    if (peerInfoHash != tf_.infoHash) {
        throw std::runtime_error("PerformHandshake: info_hash mismatch");
    }
    peerId_ = response.substr(1 + 19 + 8 + 20, 20);
}

bool PeerConnect::EstablishConnection() {
    try {
        PerformHandshake();
        ReceiveBitfield();
        SendInterested();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to establish connection with peer " << socket_.GetIp() << ":" <<
            socket_.GetPort() << " -- " << e.what() << std::endl;
        return false;
    }
}

void PeerConnect::ReceiveBitfield() {
    while (true) {
        std::string data = socket_.ReceiveData();
        Message message = Message::Parse(data);
        switch (message.id) {
            case MessageId::BitField: {
                piecesAvailability_ = PeerPiecesAvailability(message.payload);
                choked_ = true;
                return;
            }
            case MessageId::Unchoke: {
                choked_ = false;
                return;
            }
            case MessageId::KeepAlive: {
                continue;
            }
            default: {
                continue;
            }
        }
    }
}

void PeerConnect::SendInterested() {
    Message message = Message::Init(MessageId::Interested, "");
    socket_.SendData(message.ToString());
}

void PeerConnect::Terminate() {
    std::cerr << "Terminate" << std::endl;
    terminated_ = true;
}

void PeerConnect::MainLoop() {
    /*
     * При проверке вашего решения на сервере этот метод будет переопределен.
     * Если вы провели handshake верно, то в этой функции будет работать обмен данными с пиром
     */
    std::cout << "Dummy main loop" << std::endl;
    Terminate();
}


