#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

inline HttpMethod StringToMethod(const std::string& str) {
    if (str == "GET") {
        return HttpMethod::GET;
    } 

    if (str == "POST") {
        return HttpMethod::POST;
    }

    if (str == "PUT") {
        return HttpMethod::PUT;
    }

    if (str == "DELETE") {
        return HttpMethod::DELETE;
    }

    std::ostringstream msg;
    msg << "String to method error: " << str << " is unknown method";
    throw std::runtime_error(msg.str());
}