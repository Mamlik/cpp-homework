#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>

namespace {
constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash) 
    : index_(index),
      length_(length),
      hash_(std::move(hash)) {
    blocks_.resize(length / BLOCK_SIZE + (length % BLOCK_SIZE != 0 ? 1 : 0));    
    for (size_t i = 0; i < blocks_.size(); ++i) {
        blocks_[i].piece = index_;
        blocks_[i].offset = i * BLOCK_SIZE;
        blocks_[i].length = std::min(BLOCK_SIZE, length_ - i * BLOCK_SIZE);
        blocks_[i].status = Block::Missing;
    }
}

bool Piece::HashMatches() const {
    return hash_ == CalculateSHA1(GetData());
}

Block* Piece::FirstMissingBlock() {
    for (auto& block : blocks_) {
        if (block.status == Block::Missing) {
            return &block;
        }
    }
    return nullptr;
}

size_t Piece::GetIndex() const {
    return index_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data) {
    for (auto& block : blocks_) {
        if (block.offset == blockOffset) {
            if (data.size() != block.length) {
                throw std::runtime_error("SaveBlock: incorrect data length");
            }
            block.data = std::move(data);
            block.status = Block::Retrieved;
            return;
        }
    }
    throw std::runtime_error("SaveBlock: block not found");
}

bool Piece::AllBlocksRetrieved() const {
    for (auto& block : blocks_) {
        if (block.status != Block::Retrieved) {
            return false;
        }
    }
    return true;
}

std::string Piece::GetData() const {
    std::string allData;
    for (const auto& block : blocks_) {
        allData += block.data;
    }
    return allData;
}

std::string Piece::GetDataHash() const {
    return CalculateSHA1(GetData());
}

const std::string& Piece::GetHash() const {
    return hash_;
}

void Piece::Reset() {
    for (auto& block : blocks_) {
        block.status = Block::Missing;
        block.data.clear();
    }
}