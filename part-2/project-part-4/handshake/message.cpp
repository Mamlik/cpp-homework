#include "message.h"
#include <iostream>
#include "byte_tools.h"
#include <stdexcept>


Message Message::Parse(const std::string& messageString) {
    if(messageString.empty()) {
        return {MessageId::KeepAlive, 0, ""};
    }
    else {
        return {static_cast<MessageId>(messageString.front()),
                messageString.size(),
                messageString.substr(1)};
    }
}

Message Message::Init(MessageId id, const std::string& payload) {
    return Message{id, payload.size() + 1, payload};
}

std::string Message::ToString() const {
    std::string out;
    out.resize(4);
    uint32_t len = static_cast<uint32_t>(messageLength);
    out[0] = static_cast<char>((len >> 24) & 0xFF);
    out[1] = static_cast<char>((len >> 16) & 0xFF);
    out[2] = static_cast<char>((len >> 8) & 0xFF);
    out[3] = static_cast<char>(len & 0xFF);
    out += static_cast<char>(static_cast<uint8_t>(id));
    out += payload;
    return out;
}