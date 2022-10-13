#pragma once
#include <string>
#include "SockStream.h"


struct WSPacket {
    enum { continuation = 0x0, text = 0x1, binary = 0x2, connection_close = 0x8, ping = 0x9, pong = 0xA };
    uint8_t opcode;
    std::string data;
};


class WebsocketConnection : private SockConnection {
public:
    size_t singleSendThreshold = 256 * 1024;
    const bool isServer;

    WebsocketConnection(SockConnection&& s, bool isServer) : SockConnection(std::move(s)), isServer(isServer) { }
    WebsocketConnection(const SockConnection& s, bool isServer) = delete;

    WSPacket Receive();
    ///singleSend true: data copied, sent in one call
    ///singleSend false: data not copied, send() called twice
    void Send(std::string_view data, uint8_t opcode = WSPacket::text, std::optional<bool> singleSend = std::nullopt);
    void Send(const WSPacket& packet, std::optional<bool> singleSend = std::nullopt) {
        return Send(packet.data, packet.opcode, singleSend);
    }

    friend bool operator<(const WebsocketConnection& a, const WebsocketConnection& b) { return a.sock < b.sock; }
};





// Full websocket protocol info
// tools.ietf.org/html/rfc6455#section-5.2

/*  FULL

  /-----byte----\ /-----byte----\ /-----byte----\ /-----byte----\
  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|  (7) bits   |        (16 / 64) bits         |
 |N|V|V|V|       |S|             | (if payload len == 126 / 127) |
 | |1|2|3|       |K|    (PL1)    |            (PL2)              |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |   Extended payload length continued, if payload len == 127    |
 | - - - -   (64) bits   - - - - +-------------------------------+
 |              (PL3)            | Masking-key, if MASK set to 1 |
 +-------------------------------+-------------------------------+
 |    Masking-key (continued)    |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+

*/


/*
 FIN: final fragment in a message
 RSV: reserved for extensions, should be zero unless stated otherwise
 opcode:
      *  %x0 denotes a continuation frame
      *  %x1 denotes a text frame
      *  %x2 denotes a binary frame
      *  %x3-7 are reserved for further non-control frames
      *  %x8 denotes a connection close
      *  %x9 denotes a ping
      *  %xA denotes a pong
      *  %xB-F are reserved for further control frames

 MASK: must be 1 for client->server messages
 MASK: must be 0 for server->client messages
 Payload length (in bytes):
      +-------+-------------------+---------------------------+
      |  PL1  |  Payload length   | Interpretation            |
      +-------+-------------------+---------------------------+
      | 0-125 | 0-125             | literally                 |
      |  126  | 16 bit of PL2     | uint16                    |
      |  127  | 64 bit of PL2+PL3 | uint64, but MSB must be 0 |
      +-------+-------------------+---------------------------+

 Payload Data: Extension data (0 unless stated otherwise) + Actual data

*/



/*  WHEN PAYLOAD LENGTH IS 0-125

  /-------------\ /-------------\
  0 1 2 3 4 5 6 7 8 9 A B C D E F
 +-+-+-+-+-------+-+-------------+
 |F|R|R|R| opcode|M| Payload len |
 |I|S|S|S|  (4)  |A|     (7)     |
 |N|V|V|V|       |S|   (0-125)   |
 | |1|2|3|       |K|             |
 +-+-+-+-+-------+-+-------------+
 |    Mask[0]    |    Mask[1]    |
 + - - - - - - - - - - - - - - - +
 |    Mask[2]    |    Mask[3]    |
 +-------------------------------+
 |          Payload Data         |
 +-------------------------------+

*/



/*  diep.io client->server packet

  /-------------\ /-------------\
  0 1 2 3 4 5 6 7 8 9 A B C D E F

 +-+-+-+-+-------+-+-------------+
 |1|0|0|0|0 0 1 0|1| Payload len |
 +-+-+-+-+-------+-+-------------+
 |    Mask[0]    |    Mask[1]    |
 + - - - - - - - - - - - - - - - +
 |    Mask[2]    |    Mask[3]    |
 +-------------------------------+
 |    Payload[i] ^ Mask[i%4]     |


*/
