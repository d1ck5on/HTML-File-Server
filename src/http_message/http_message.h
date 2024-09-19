#pragma once

#include <istream>
#include <list>
#include <memory>
#include <utils/http_version.h>
#include <unordered_map>

class HttpMessage {
public:
    virtual ~HttpMessage() = default;

    HttpVersion Version() const;
    const std::string& Header(const std::string& key) const;
    bool HasHeader(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& Headers() const;
    std::list<std::unique_ptr<std::iostream>>& Body();

    void SetHeader(const std::string& key, const std::string& value);
    void AddBody(std::unique_ptr<std::iostream>&& body);
    void AddSize(uintmax_t length);
    void SetVersion(HttpVersion version);

protected:
    HttpVersion version_;
    std::unordered_map<std::string, std::string> headers_;
    std::list<std::unique_ptr<std::iostream>> body_;
    uintmax_t content_length_ = 0;
};