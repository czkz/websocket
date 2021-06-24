#include <iostream>
#include <thread>
#include <span>
#include "SockStream.h"

#define ENABLE_GET_HTTP
#include "http.h"
#undef ENABLE_GET_HTTP
#include "sha1/sha1.hpp"
#include "base64.h"
#include "Websocket.h"

#include "leet.h"

static constexpr size_t MAX_MSG_LENGTH = -1;

void foo(SockConnection&& cl);

int main() {
    SockServer server;
	server.Start(1337);

    while(true) {
        ClientConnection cl = server.Accept();
        std::cout << "Client connected" << std::endl;
        std::thread(foo, std::move(cl)).detach();
    }
}

void foo(SockConnection&& cl) try {
    std::string key;
    {  //HTTP request
        std::string s = http::recv_head(cl);

        http::http_view hv(s);

        if (!hv.has("Connection", "Upgrade")) { return; }
        if (!hv.has("Upgrade", "websocket")) { return; }

        key = hv.get_all("Sec-WebSocket-Key");

        if (key.empty()) { return; }
    }

    {  //HTTP response
        const std::string keyResponse = to_base64(SHA1(std::string(key) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").final());

        http::packet response("HTTP/1.1 101 Switching Protocols");
        response.push("Connection", "Upgrade");
        response.push("Upgrade", "websocket");
        response.push("Sec-WebSocket-Accept", keyResponse);

        cl.Send(response.str());
    }

    WebsocketConnection ws {std::move(cl), true};
    Leet leet;

    //Main loop
    while(true) {
        WSPacket p = ws.Receive();
        std::cout << "Received " << p.data.size() << " bytes" << std::endl;
        if (p.opcode == p.connection_close) {
            std::cout << "Received connection_close" << std::endl;
            break;
        }
        const auto out = leet.encode(p.data);
        std::span out_span (out);
        std::string_view out_sv ((char*) std::as_bytes(out_span).data(), out_span.size_bytes());
        ws.Send(out_sv);
    }

} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
}
