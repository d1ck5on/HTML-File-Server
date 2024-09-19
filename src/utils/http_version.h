#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

enum class HttpVersion {
    HTTP_0_9 = 9,
    HTTP_1_0 = 10,
    HTTP_1_1 = 11,
    HTTP_2_0 = 20
};

inline HttpVersion StringToVersion(const std::string& str) {
    if (str == "HTTP/0.9") {
        return HttpVersion::HTTP_0_9;
    } 

    if (str == "HTTP/1.0") {
        return HttpVersion::HTTP_1_0;
    } 

    if (str == "HTTP/1.1") {
        return HttpVersion::HTTP_1_1;
    } 

    if (str == "HTTP/2.0") {
        return HttpVersion::HTTP_2_0;
    } 

    std::ostringstream msg;
    msg << "String to version error: " << str << " is unknown version";
    throw std::runtime_error(msg.str());
}

inline std::string VersionToString(HttpVersion& version) {
    switch (version) {
        case HttpVersion::HTTP_0_9:
            return "HTTP/0.9";
        case HttpVersion::HTTP_1_0:
            return "HTTP/1.0";
        case HttpVersion::HTTP_1_1:
            return "HTTP/1.1";
        case HttpVersion::HTTP_2_0:
            return "HTTP/2.0";
        default:
            throw std::runtime_error("Unknown http version");
    }
}