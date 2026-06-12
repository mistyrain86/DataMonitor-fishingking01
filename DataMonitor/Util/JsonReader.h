#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

namespace JsonReader {

using JsonObject = std::map<std::string, std::string>;
using JsonArray  = std::vector<JsonObject>;

inline void skipWS(const std::string& s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
        ++i;
}

inline std::string parseStr(const std::string& s, size_t& i) {
    if (i >= s.size() || s[i] != '"') return {};
    ++i;
    std::string r;
    while (i < s.size() && s[i] != '"') {
        if (s[i] == '\\') {
            ++i;
            if (i >= s.size()) break;
            char c = s[i];
            if      (c == '"')  r += '"';
            else if (c == '\\') r += '\\';
            else if (c == '/')  r += '/';
            else if (c == 'n')  r += '\n';
            else if (c == 'r')  r += '\r';
            else if (c == 't')  r += '\t';
            else if (c == 'u') {
                if (i + 4 < s.size()) {
                    unsigned int cp = 0;
                    for (int k = 1; k <= 4; ++k) {
                        cp <<= 4;
                        char h = s[i + k];
                        if      (h >= '0' && h <= '9') cp |= (h - '0');
                        else if (h >= 'a' && h <= 'f') cp |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') cp |= (h - 'A' + 10);
                    }
                    i += 4;
                    if (cp < 0x80u) {
                        r += (char)cp;
                    } else if (cp < 0x800u) {
                        r += (char)(0xC0u | (cp >> 6));
                        r += (char)(0x80u | (cp & 0x3Fu));
                    } else {
                        r += (char)(0xE0u | (cp >> 12));
                        r += (char)(0x80u | ((cp >> 6) & 0x3Fu));
                        r += (char)(0x80u | (cp & 0x3Fu));
                    }
                }
            } else {
                r += c;
            }
        } else {
            r += s[i];
        }
        ++i;
    }
    if (i < s.size()) ++i;
    return r;
}

inline void skipVal(const std::string& s, size_t& i);

inline JsonObject parseObj(const std::string& s, size_t& i) {
    JsonObject obj;
    if (i >= s.size() || s[i] != '{') return obj;
    ++i;
    skipWS(s, i);
    while (i < s.size() && s[i] != '}') {
        skipWS(s, i);
        if (i >= s.size() || s[i] != '"') break;
        std::string key = parseStr(s, i);
        skipWS(s, i);
        if (i < s.size() && s[i] == ':') ++i;
        skipWS(s, i);
        if (i >= s.size()) break;
        if (s[i] == '"') {
            obj[key] = parseStr(s, i);
        } else if (s[i] == '{' || s[i] == '[') {
            skipVal(s, i);
        } else {
            size_t start = i;
            while (i < s.size() && s[i] != ',' && s[i] != '}' && s[i] != ']'
                   && s[i] != ' ' && s[i] != '\n' && s[i] != '\r' && s[i] != '\t')
                ++i;
            obj[key] = s.substr(start, i - start);
        }
        skipWS(s, i);
        if (i < s.size() && s[i] == ',') ++i;
        skipWS(s, i);
    }
    if (i < s.size() && s[i] == '}') ++i;
    return obj;
}

inline void skipVal(const std::string& s, size_t& i) {
    if (i >= s.size()) return;
    if (s[i] == '"') { parseStr(s, i); return; }
    if (s[i] == '{') {
        int d = 0;
        while (i < s.size()) {
            if      (s[i] == '{') ++d;
            else if (s[i] == '}') { --d; if (!d) { ++i; return; } }
            else if (s[i] == '"') { parseStr(s, i); continue; }
            ++i;
        }
        return;
    }
    if (s[i] == '[') {
        int d = 0;
        while (i < s.size()) {
            if      (s[i] == '[') ++d;
            else if (s[i] == ']') { --d; if (!d) { ++i; return; } }
            else if (s[i] == '"') { parseStr(s, i); continue; }
            ++i;
        }
        return;
    }
    while (i < s.size() && s[i] != ',' && s[i] != '}' && s[i] != ']'
           && s[i] != ' ' && s[i] != '\n' && s[i] != '\r' && s[i] != '\t')
        ++i;
}

inline JsonArray parseArr(const std::string& s, size_t& i) {
    JsonArray arr;
    if (i >= s.size() || s[i] != '[') return arr;
    ++i;
    skipWS(s, i);
    while (i < s.size() && s[i] != ']') {
        if (s[i] == '{') arr.push_back(parseObj(s, i));
        else skipVal(s, i);
        skipWS(s, i);
        if (i < s.size() && s[i] == ',') ++i;
        skipWS(s, i);
    }
    if (i < s.size()) ++i;
    return arr;
}

inline JsonArray readArray(const std::string& filePath, const std::string& key) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return {};
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string c = ss.str();
    if (c.size() >= 3u &&
        (unsigned char)c[0] == 0xEFu &&
        (unsigned char)c[1] == 0xBBu &&
        (unsigned char)c[2] == 0xBFu)
        c = c.substr(3);

    size_t i = 0;
    skipWS(c, i);
    if (i >= c.size() || c[i] != '{') return {};
    ++i;
    while (i < c.size()) {
        skipWS(c, i);
        if (i >= c.size() || c[i] == '}') break;
        if (c[i] != '"') { ++i; continue; }
        std::string k = parseStr(c, i);
        skipWS(c, i);
        if (i < c.size() && c[i] == ':') ++i;
        skipWS(c, i);
        if (k == key && i < c.size() && c[i] == '[')
            return parseArr(c, i);
        skipVal(c, i);
        skipWS(c, i);
        if (i < c.size() && c[i] == ',') ++i;
    }
    return {};
}

} // namespace JsonReader
