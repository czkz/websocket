#pragma once
#include <string>
#include <string_view>

std::string to_base64(std::string_view data) {
    static constexpr std::string_view baked[256] = {
        "AAAA", "AQEB", "AgIC", "AwMD", "BAQE", "BQUF", "BgYG", "BwcH",
        "CAgI", "CQkJ", "CgoK", "CwsL", "DAwM", "DQ0N", "Dg4O", "Dw8P",
        "EBAQ", "ERER", "EhIS", "ExMT", "FBQU", "FRUV", "FhYW", "FxcX",
        "GBgY", "GRkZ", "Ghoa", "Gxsb", "HBwc", "HR0d", "Hh4e", "Hx8f",
        "ICAg", "ISEh", "IiIi", "IyMj", "JCQk", "JSUl", "JiYm", "Jycn",
        "KCgo", "KSkp", "Kioq", "Kysr", "LCws", "LS0t", "Li4u", "Ly8v",
        "MDAw", "MTEx", "MjIy", "MzMz", "NDQ0", "NTU1", "NjY2", "Nzc3",
        "ODg4", "OTk5", "Ojo6", "Ozs7", "PDw8", "PT09", "Pj4+", "Pz8/",
        "QEBA", "QUFB", "QkJC", "Q0ND", "RERE", "RUVF", "RkZG", "R0dH",
        "SEhI", "SUlJ", "SkpK", "S0tL", "TExM", "TU1N", "Tk5O", "T09P",
        "UFBQ", "UVFR", "UlJS", "U1NT", "VFRU", "VVVV", "VlZW", "V1dX",
        "WFhY", "WVlZ", "Wlpa", "W1tb", "XFxc", "XV1d", "Xl5e", "X19f",
        "YGBg", "YWFh", "YmJi", "Y2Nj", "ZGRk", "ZWVl", "ZmZm", "Z2dn",
        "aGho", "aWlp", "ampq", "a2tr", "bGxs", "bW1t", "bm5u", "b29v",
        "cHBw", "cXFx", "cnJy", "c3Nz", "dHR0", "dXV1", "dnZ2", "d3d3",
        "eHh4", "eXl5", "enp6", "e3t7", "fHx8", "fX19", "fn5+", "f39/",
        "gICA", "gYGB", "goKC", "g4OD", "hISE", "hYWF", "hoaG", "h4eH",
        "iIiI", "iYmJ", "ioqK", "i4uL", "jIyM", "jY2N", "jo6O", "j4+P",
        "kJCQ", "kZGR", "kpKS", "k5OT", "lJSU", "lZWV", "lpaW", "l5eX",
        "mJiY", "mZmZ", "mpqa", "m5ub", "nJyc", "nZ2d", "np6e", "n5+f",
        "oKCg", "oaGh", "oqKi", "o6Oj", "pKSk", "paWl", "pqam", "p6en",
        "qKio", "qamp", "qqqq", "q6ur", "rKys", "ra2t", "rq6u", "r6+v",
        "sLCw", "sbGx", "srKy", "s7Oz", "tLS0", "tbW1", "tra2", "t7e3",
        "uLi4", "ubm5", "urq6", "u7u7", "vLy8", "vb29", "vr6+", "v7+/",
        "wMDA", "wcHB", "wsLC", "w8PD", "xMTE", "xcXF", "xsbG", "x8fH",
        "yMjI", "ycnJ", "ysrK", "y8vL", "zMzM", "zc3N", "zs7O", "z8/P",
        "0NDQ", "0dHR", "0tLS", "09PT", "1NTU", "1dXV", "1tbW", "19fX",
        "2NjY", "2dnZ", "2tra", "29vb", "3Nzc", "3d3d", "3t7e", "39/f",
        "4ODg", "4eHh", "4uLi", "4+Pj", "5OTk", "5eXl", "5ubm", "5+fn",
        "6Ojo", "6enp", "6urq", "6+vr", "7Ozs", "7e3t", "7u7u", "7+/v",
        "8PDw", "8fHx", "8vLy", "8/Pz", "9PT0", "9fX1", "9vb2", "9/f3",
        "+Pj4", "+fn5", "+vr6", "+/v7", "/Pz8", "/f39", "/v7+", "////"
    };
    std::basic_string_view<uint8_t> s = { (uint8_t*) data.data(), data.size() };
    std::string ret;
    const size_t origLen = s.size();
    const size_t newLen = (origLen / 3 + (origLen%3!=0)) * 4;
    ret.reserve(newLen);

    size_t i;
    for (i = 0; i < origLen - 2; i += 3) {
        ret += baked[  s[i]                                         ][0];
        ret += baked[ (s[i+0] & 0b00000011) | (s[i+1] & 0b11110000) ][1];
        ret += baked[ (s[i+1] & 0b00001111) | (s[i+2] & 0b11000000) ][2];
        ret += baked[  s[i+2]                                       ][3];
    }
    switch (origLen % 3) {
    case 2:
        ret += baked[  s[i]                                         ][0];
        ret += baked[ (s[i+0] & 0b00000011) | (s[i+1] & 0b11110000) ][1];
        ret += baked[  s[i+1] & 0b00001111                          ][2];
        break;
    case 1:
        ret += baked[  s[i]                                         ][0];
        ret += baked[  s[i+0] & 0b00000011                          ][1];
        break;
    }
    ret.resize(newLen, '=');
    return ret;
}
