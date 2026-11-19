#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>

using namespace std::chrono_literals;



PeerConnect::PeerConnect(const Peer& peer, const TorrentFile& tf, std::string selfPeerId, PieceStorage& pieceStorage)
    : tf_(tf),
      socket_(peer.ip, peer.port, 7s, 7s),
      selfPeerId_(std::move(selfPeerId)),
      terminated_(false),
      choked_(true),
      pieceStorage_(pieceStorage),
      pendingBlock_(false) {
}

void PeerConnect::Run() {
    if (pieceStorage_.QueueIsEmpty()) {
        std::cerr << "Queue is empty" << std::endl;
        Terminate();
        return;
    }
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            try {
                MainLoop();
            } catch (const std::exception& e) {
                std::cerr << "Error in main loop: " << e.what() << std::endl;
                Terminate();
            }
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

void PeerConnect::RequestPiece() {
    if (!pieceInProgress_) {
        pieceInProgress_ = pieceStorage_.GetNextPieceToDownload();
        if (!pieceInProgress_) {
            return;
        }
        if (!piecesAvailability_.IsPieceAvailable(pieceInProgress_->GetIndex())) {
            pieceInProgress_.reset();
            return;
        }
    }
    Block* block = pieceInProgress_->FirstMissingBlock();
    if (!block) {
        if (pieceInProgress_->AllBlocksRetrieved()) {
            std::cout << "Piece " << pieceInProgress_->GetIndex() << " downloaded successfully" << std::endl;
            pieceInProgress_.reset();
        }
        return;
    }
    std::string payload;
    payload.reserve(12);
    payload.push_back(static_cast<char>((block->piece >> 24) & 0xFF));
    payload.push_back(static_cast<char>((block->piece >> 16) & 0xFF));
    payload.push_back(static_cast<char>((block->piece >> 8) & 0xFF));
    payload.push_back(static_cast<char>( block->piece        & 0xFF));
    payload.push_back(static_cast<char>((block->offset >> 24) & 0xFF));
    payload.push_back(static_cast<char>((block->offset >> 16) & 0xFF));
    payload.push_back(static_cast<char>((block->offset >> 8) & 0xFF));
    payload.push_back(static_cast<char>( block->offset        & 0xFF));
    payload.push_back(static_cast<char>((block->length >> 24) & 0xFF));
    payload.push_back(static_cast<char>((block->length >> 16) & 0xFF));
    payload.push_back(static_cast<char>((block->length >> 8) & 0xFF));
    payload.push_back(static_cast<char>( block->length        & 0xFF));
    Message req = Message::Init(MessageId::Request, payload);
    socket_.SendData(req.ToString());
    pendingBlock_ = true;
}

void PeerConnect::Terminate() {
    std::cerr << "Terminate" << std::endl;
    terminated_ = true;
}

void PeerConnect::MainLoop() {
    while (!terminated_) {
        if (pieceStorage_.QueueIsEmpty()) {
            std::cerr << "Queue is empty" << std::endl;
            Terminate();
            return;
        }
        std::string data = socket_.ReceiveData();
        Message message = Message::Parse(data);
        switch (message.id) {
            case MessageId::Choke: {
                choked_ = true;
                break;
            }
            case MessageId::Unchoke: {
                choked_ = false;
                break;
            }
            case MessageId::Have: {
                uint32_t pieceIndex = BytesToInt({message.payload.data(), 4});
                piecesAvailability_.SetPieceAvailability(pieceIndex);
                break;
            }
            case MessageId::Piece: {
                if (pieceInProgress_) {
                    uint32_t pieceIndex = BytesToInt({message.payload.data(), 4});
                    uint32_t blockOffset = BytesToInt({message.payload.data() + 4, 4});
                    std::string blockData = message.payload.substr(8);
                    if (pieceInProgress_ && pieceInProgress_->GetIndex() == pieceIndex) {
                        pieceInProgress_->SaveBlock(blockOffset, blockData);
                        if (pieceInProgress_->AllBlocksRetrieved()) {
                            if (pieceInProgress_->HashMatches()) {
                                pieceStorage_.PieceProcessed(pieceInProgress_);
                            }
                            pieceInProgress_.reset();
                        }
                    }
                }
                pendingBlock_ = false;
                break;
            }

            default: {
                break;
            }
        }
        if (!choked_ && !pendingBlock_) {
            RequestPiece();
        }
    }
}
