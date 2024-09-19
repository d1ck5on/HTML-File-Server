#pragma once 

#include <http_message/http_message.h>
#include <memory>
#include <utils/http_method.h>

class HttpRequest : public HttpMessage {
public:
    void ParseFromStream(std::unique_ptr<std::iostream>& stream);
    HttpMethod Method() const;
    const std::string& Uri() const;

    void SetMethod(const HttpMethod& method);
    void SetUri(const std::string& uri);

    ~HttpRequest() override = default;

private:
    HttpMethod method_;
    std::string uri_;
};