#pragma once

#include <file_manager.h>
#include <server.h>
#include <http_message/http_request.h>
#include <http_message/http_response.h>


#include <string>

class HttpServer {
public:
    explicit HttpServer(std::string host, std::uint16_t port, const std::string& working_dir, 
    const std::string& server_name = "server", int backlog_size = 10);

    void Start();

    void HandleRequest(std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response);

    void Get(const HttpRequest& request, HttpResponse& response);
    void Post(const HttpRequest& request, HttpResponse& response);
    void Put(HttpRequest& request, HttpResponse& response);
    void Delete(const HttpRequest& request, HttpResponse& response);

private:
    std::string server_name_;
    FileManager file_manager_;
    
    Server server_;
};