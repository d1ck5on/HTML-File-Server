#include <cstdio>
#include <http_message/http_response.h>
#include <glog/raw_logging.h>

#include <istream>
#include <memory>
#include <sstream>
#include <string>

HttpStatusCode HttpResponse::StatusCode() {
    return status_code_;
}

void HttpResponse::SetStatusCode(HttpStatusCode status_code) {
    status_code_ = status_code;
}

std::list<std::unique_ptr<std::iostream>> HttpResponse::ToStreams() {
    std::stringstream buf;
    buf << content_length_;
    headers_["Content-Length"] = buf.str();
    std::list<std::unique_ptr<std::iostream>> response;

    std::unique_ptr<std::stringstream> head = std::make_unique<std::stringstream>();

    *head << VersionToString(version_) << " " << std::to_string(int(status_code_)) << " "
    << StatusToString(status_code_) << std::endl;

    for (auto [key, value] : headers_) {
        *head << key << ": " << value << std::endl;
    }

    *head << std::endl;

    RAW_LOG_INFO("%s", head->str().c_str());
    response.push_back(std::move(head));

    for (auto& str : body_) {
        response.push_back(std::move(str));
    }

    return response;
}