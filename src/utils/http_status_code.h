#pragma once

#include <stdexcept>
#include <string>

enum class HttpStatusCode {
    OK = 200,
    CREATED = 201,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    NOT_ACCEPTABLE = 406,
    CONFLICT = 409,
    NOT_IMPLEMENTED = 501,
};

inline std::string StatusToString(HttpStatusCode status) {
    switch (status) {
        case HttpStatusCode::OK:
            return "OK";
        case HttpStatusCode::CREATED:
            return "CREATED";
        case HttpStatusCode::BAD_REQUEST:
            return "BAD REQUEST";
        case HttpStatusCode::NOT_FOUND:
            return "NOT FOUND";
        case HttpStatusCode::NOT_ACCEPTABLE:
            return "NOT ACCEPTABLE";
        case HttpStatusCode::CONFLICT:
            return "CONFLICT";
        case HttpStatusCode::NOT_IMPLEMENTED:
            return "NOT IMPLEMENTED";
        default:
            throw std::runtime_error("Unknown http status");
    }
}