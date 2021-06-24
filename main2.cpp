#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include "SockStream.h"

#define ENABLE_GET_HTTP
#include "http.h"
#undef ENABLE_GET_HTTP
#include "sha1/sha1.hpp"
#include "base64.h"
#include "Websocket.h"

using std::cout;

void sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::set<WebsocketConnection> clients;
std::mutex clients_mutex;

using set_it = decltype(clients)::iterator;

void onReceived(std::string data, set_it who) {
    std::lock_guard l { clients_mutex };
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it == who) { continue; }
        try {
            it->Send(data);
        } catch (const SockError& err) {
            std::cout << err.what() << std::endl;
        }
    }
}

void clientThreadFunc(SockConnection&& cl) {
    set_it it;
    try {
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

        {
            WebsocketConnection ws {std::move(cl), true};
            std::lock_guard l { clients_mutex };
            it = clients.insert(std::move(ws)).first;
        }

        // while (true) {
        // }
        //Main loop
        while(true) {
            WSPacket p = it->Receive();
            std::cout << "Received " << p.data.size() << " bytes" << std::endl;
            if (p.opcode == p.connection_close) {
                std::cout << "Received connection_close" << std::endl;
                break;
            }
            onReceived(p.data, it);
        }
    } catch (const SockError& e) {
        if (it != set_it()) {
            std::lock_guard l { clients_mutex };
            clients.erase(it);
        }
    }
}

int main() {
    SockServer server;
    server.Start(1337);

    while (true) {
        auto c = server.Accept();
        std::thread(clientThreadFunc, std::move(c)).detach();
    }
}
