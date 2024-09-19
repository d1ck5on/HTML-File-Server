#pragma once

#include <http_message/http_message.h>
#include <utils/http_status_code.h>

class HttpResponse : public HttpMessage {
public:
    std::list<std::unique_ptr<std::iostream>> ToStreams();

    HttpStatusCode StatusCode();

    void SetStatusCode(HttpStatusCode status_code);

    ~HttpResponse() override = default;

private:
    HttpStatusCode status_code_;
};