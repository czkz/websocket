#pragma once
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include "SockStream.h"

namespace http {

    ///case-sensitive
    class http_view {
    public:
        const std::string_view full;
        std::string_view request;
        std::string_view headers;
        std::string_view body;
        constexpr http_view(std::string_view full) : full(std::move(full)) {
            const auto p1 = full.find("\r\n");
            auto p2 = full.find("\r\n\r\n");
            request = full.substr(0, p1);
            if (p2 != std::string_view::npos) {
                headers = full.substr(p1 + 2, p2 - p1);
                body = full.substr(p2 + 4);
            } else {
                headers = full.substr(p1 + 2);
                body = full.substr(full.size());
            }
        }
        constexpr std::string_view getRequest() const {
            return request;
        }
        constexpr std::string_view getBody() const {
            return body;
        }
        bool has(std::string_view header, std::string_view value = "") const {
            return find_line(header).find(value) != std::string::npos;
        }
        std::vector<std::string_view> get(std::string_view header) const {
            using std::string;
            const auto line = find_line(header);
            string::size_type pos1 = 0;
            string::size_type pos2;
            std::vector<std::string_view> v;
            while (pos1 < line.size()) {
                if (line[pos1] == ' ') { pos1++; }
                pos2 = line.find(",", pos1);
                if (pos2 == string::npos) { pos2 = line.size(); }
                v.emplace_back(line.data() + pos1, pos2 - pos1);
                pos1 = pos2 + 1;
            }
            return v;
        }
        std::string_view get_all(std::string_view header) const {
            return find_line(header);
        }

    private:
        std::string_view find_line(std::string_view header) const {
            std::string a1 = "\r\n";
            a1 += header;
            a1 += ": ";
            auto i1 = headers.find(a1);
            if (i1 == std::string::npos) {
                return std::string_view();
            }
            i1 += a1.size();
            auto i2 = headers.find("\r\n", i1);
            if (i2 == std::string::npos) {
                throw std::out_of_range("Invalid http request");
            }
            return std::string_view(headers.data() + i1, i2 - i1);
        }
    };



    class packet {
    public:
        packet(std::string request) : request(std::move(request)) { }
        std::string request;
        std::string body;
        std::map<std::string, std::vector<std::string>, std::less<>> headers;
        void push(const std::string& header, std::string value) {
            headers[header].push_back(std::move(value));
        }
        void push(std::string_view full) {
            std::string_view header = full.substr(0, full.find(":"));
            std::string_view value = full.substr(full.find(":") + 1);
            if (value[0] == ' ') {
                value = full.substr(full.find(":") + 2);
            }
            headers[(std::string) header].push_back(std::string(value));
        }
        std::string str() {
            std::string s = request;
            s += "\r\n";
            for (const auto& e : headers) {
                s += e.first;
                s += ": ";
                for (const auto& v : e.second) {
                    s += v;
                    s += ", ";
                }
                s.resize(s.size() - 2);
                s += "\r\n";
            }
            s += "\r\n";
            s += body;
            return s;
        }
        void clear() {
            headers.clear();
        }
    };



    std::string recv_head(SockConnection& sock) {
        std::string s;
        do {
            s += sock.ReceiveAvailable();
        } while (s.find("\r\n\r\n") == std::string::npos);
        return s;
    }
}
