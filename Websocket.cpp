#include "Websocket.h"

///FIXME
#include <iostream>

template <typename T>
static void swapEndian(T& val) {
    char* raw = (char*) &val;
    constexpr int len = sizeof(val);
    for (int i = 0; i < len / 2; i++) {
        std::swap(raw[i], raw[len - i - 1]);
    }
}

///TODO handle opcode
WSPacket WebsocketConnection::Receive() const {
    std::string s = SockConnection::ReceiveFill(2);
    bool    fin         = s[0] & 0b10000000;
    // bool    rsv1        = s[0] & 0b01000000;
    // bool    rsv2        = s[0] & 0b00100000;
    // bool    rsv3        = s[0] & 0b00010000;
    uint8_t opcode      = s[0] & 0b00001111;
    bool    mask        = s[1] & 0b10000000;
    uint8_t payload_len = s[1] & 0b01111111;

    uint64_t actualPayloadLen = payload_len;
    if (payload_len == 126) {
        uint16_t payload_len16;
        SockConnection::ReceiveInto(&payload_len16, sizeof payload_len16);
        swapEndian(payload_len16);
        actualPayloadLen = payload_len16;
    } else if (payload_len == 127) {
        uint64_t payload_len64;
        SockConnection::ReceiveInto(&payload_len64, sizeof payload_len64);
        swapEndian(payload_len64);
        actualPayloadLen = payload_len64;
    } else {
        actualPayloadLen = payload_len;
    }

    uint8_t masking_key[4] = {0};
    if (mask) {
        SockConnection::ReceiveInto(masking_key, 4);
    }

    std::string data = SockConnection::ReceiveFill(actualPayloadLen);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] ^= masking_key[i % 4];
    }
    //To prevent stack overflow exploits, tcp doesn't use that anyway
    if (false && fin == false) {
        ///TODO test
        std::cout << "WebsocketConnection::Receive() recursing" << std::endl;
        WSPacket next = Receive();
        if (next.opcode != WSPacket::continuation) {
            ///TODO handle rfc6455 section-5.5
            ///TODO proper exception
            throw;
        }
        data += next.data;
    }
    return { opcode, data };
}

struct wsFlags {
    bool    fin;
    bool    rsv1;
    bool    rsv2;
    bool    rsv3;
    uint8_t opcode;
    bool    mask;
    uint8_t masking_key[4];
};

static std::string WSMakeHeader(const wsFlags& flags, size_t len) {
    std::string header;
    uint8_t firstByte = 0;
    uint8_t secondByte = 0;

    firstByte |= flags.fin  << 7;
    firstByte |= flags.rsv1 << 6;
    firstByte |= flags.rsv2 << 5;
    firstByte |= flags.rsv3 << 4;
    firstByte |= flags.opcode & 0b00001111;
    header += firstByte;

    secondByte |= flags.mask << 7;
    if (len <= 125) {
        secondByte |= len;
        header += secondByte;
    } else if (len < 65536) {
        secondByte |= 126;
        header += secondByte;
        uint16_t len16 = (uint16_t) len;
        swapEndian(len16);
        header.append((char*) &len16, sizeof len16);
    } else {
        secondByte |= 127;
        header += secondByte;
        uint64_t len64 = (uint64_t) len;
        swapEndian(len64);
        header.append((char*) &len64, sizeof len64);
    }
    if (flags.mask) {
        header.append((char*) flags.masking_key, sizeof flags.masking_key);
    }
    return header;
}

void WebsocketConnection::Send(std::string_view data, uint8_t opcode, std::optional<bool> singleSend) const {

    std::string header = WSMakeHeader({1, 0, 0, 0, opcode, !isServer,
                                          {   ///FIXME
                                              static_cast<uint8_t>(rand()),
                                              static_cast<uint8_t>(rand()),
                                              static_cast<uint8_t>(rand()),
                                              static_cast<uint8_t>(rand())
                                          }
                                      },
                                      data.size());
    if (singleSend.value_or(data.size() <= singleSendThreshold)) {
        header.append(data);
        SockConnection::Send(header);
    } else {
        SockConnection::Send(header);
        SockConnection::Send(data);
    }
}
