#include <http_message/http_request.h>
#include <glog/raw_logging.h>

#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

HttpMethod HttpRequest::Method() const {
    return method_;
}

const std::string& HttpRequest::Uri() const {
    return uri_;
}

void HttpRequest::SetMethod(const HttpMethod& method) {
    method_ = method;
}

void HttpRequest::SetUri(const std::string& uri) {
    uri_ = uri;
}

void HttpRequest::ParseFromStream(std::unique_ptr<std::iostream>& stream) {
    std::iostream& str = *stream.get();
    std::string buff;
    std::getline(str, buff, ' ');

    if (str.fail() || str.eof()) {
        std::ostringstream msg;
        msg << "Method not found in request.";
        throw std::runtime_error(msg.str());
    }

    SetMethod(StringToMethod(buff));

    std::string uri_str;
    std::getline(str, buff, ' ');

    if (str.fail() || str.eof()) {
        std::ostringstream msg;
        msg << "Method not found in request.";
        throw std::runtime_error(msg.str());
    }

    SetUri(buff);

    std::getline(str, buff);

    if (str.fail() || str.eof()) {
        std::ostringstream msg;
        msg << "Method not found in request.";
        throw std::runtime_error(msg.str());
    }

    buff.pop_back();
    SetVersion(StringToVersion(buff));

    while (true) {
        std::getline(str, buff);

        if (buff.size() == 1) {
            break;
        }

        int delim = buff.find(':');
        if (delim == std::string::npos || delim + 2 >= buff.size()) {
            std::ostringstream msg;
            msg << "Header format error";
            throw std::runtime_error(msg.str());
        }

        SetHeader(buff.substr(0, delim), buff.substr(delim + 2));
    }

    AddBody(std::move(stream));

    RAW_LOG_INFO("Successfull request parsing from string.");
}