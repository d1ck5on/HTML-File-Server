#include <http_message/http_message.h>
#include <string>

HttpVersion HttpMessage::Version() const {
    return version_;
}

const std::unordered_map<std::string, std::string>& HttpMessage::Headers() const {
    return headers_;
}

const std::string& HttpMessage::Header(const std::string& key) const {
    return headers_.at(key);
}

bool HttpMessage::HasHeader(const std::string& key) const {
    return headers_.count(key) != 0;
}

std::list<std::unique_ptr<std::iostream>>& HttpMessage::Body() {
    return body_;
}

void HttpMessage::SetHeader(const std::string& key, const std::string& value) {
    if (key == "Content-Length") {
        return;
    }

    headers_[key] = value;
}

void HttpMessage::AddSize(uintmax_t length) {
    content_length_ += length;
}

void HttpMessage::AddBody(std::unique_ptr<std::iostream>&& body) {
    body_.push_back(std::move(body));
}

void HttpMessage::SetVersion(HttpVersion version) {
    version_ = version;
}