#include <cstdio>
#include <http_server.h>
#include <glog/raw_logging.h>
#include <memory>
#include <sstream>
#include <utils/http_status_code.h>
#include <stdexcept>

HttpServer::HttpServer(std::string host, std::uint16_t port, const std::string& working_dir, const std::string& server_name, int backlog_size) 
    : 
    server_name_(server_name),
    file_manager_(working_dir),
    server_(host, port, 
    [this](std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response) {
                                this->HandleRequest(request, response);
                            }, 
    backlog_size) {};

void HttpServer::Start() {
    server_.Start();
}

void HttpServer::HandleRequest(std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response) {
    HttpRequest http_request;

    HttpResponse http_response;
    http_response.SetVersion(HttpVersion::HTTP_1_0);
    http_response.SetHeader("Server", server_name_);
    http_response.SetHeader("Content-Type", "text/plain");

    try {
        http_request.ParseFromStream(request);
    } catch (std::runtime_error& err) {
        RAW_LOG_ERROR("Cannot parse a request with error: %s", err.what());
        http_response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
        response = http_response.ToStreams();
        return;
    }

    switch (http_request.Method()) {
        case HttpMethod::GET:
            Get(http_request, http_response);
            break;
        case HttpMethod::POST:
            Post(http_request, http_response);
            break;
        case HttpMethod::PUT:
            Put(http_request, http_response);
            break;
        case HttpMethod::DELETE:
            Delete(http_request, http_response);
            break;
        default:
            http_response.SetStatusCode(HttpStatusCode::NOT_IMPLEMENTED);
    }

    response = http_response.ToStreams();
}

void HttpServer::Post(const HttpRequest& request, HttpResponse& response) {
    if (request.HasHeader("Create-Directory") && request.Header("Create-Directory") == "True") {
        try {
            file_manager_.CreateDirectory(request.Uri());
        } catch (std::runtime_error& err) {
            std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
            *msg << "Cannot create directory " << request.Uri().c_str() << " with error " << err.what();
            RAW_LOG_ERROR("%s", msg->str().c_str());
            response.SetStatusCode(HttpStatusCode::CONFLICT);
            response.AddSize(msg->str().size());
            response.AddBody(std::move(msg));
            return;
        }

    } else {
        try {
            file_manager_.CreateFile(request.Uri());
        } catch (std::runtime_error& err) {
            std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
            *msg << "Cannot create file " << request.Uri().c_str() << " with error " << err.what();
            RAW_LOG_ERROR("%s", msg->str().c_str());
            response.SetStatusCode(HttpStatusCode::CONFLICT);
            response.AddSize(msg->str().size());
            response.AddBody(std::move(msg));
            return;
        }
    }

    response.SetStatusCode(HttpStatusCode::CREATED);

    RAW_LOG_INFO("Successfull handling POST request");
}

void HttpServer::Get(const HttpRequest& request, HttpResponse& response) {
    std::unique_ptr<std::iostream> body;
    uintmax_t size = 0;
    
    try {
        size = file_manager_.Get(request.Uri(), body);
    } catch (std::runtime_error& err) {
        std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
        *msg << "Cannot read file " << request.Uri().c_str() << " with error " << err.what();
        RAW_LOG_ERROR("%s", msg->str().c_str());
        response.SetStatusCode(HttpStatusCode::NOT_FOUND);
        response.AddSize(msg->str().size());
        response.AddBody(std::move(msg));
        return;
    }

    response.AddBody(std::move(body));
    response.AddSize(size);
    response.SetStatusCode(HttpStatusCode::OK);

    RAW_LOG_INFO("Successfull handling GET request");
}   

void HttpServer::Put(HttpRequest& request, HttpResponse& response) {
    try {
        file_manager_.Put(request.Uri(), request.Body());
    } catch (std::runtime_error& err) {
        std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
        *msg << "Cannot put to file " << request.Uri().c_str() << " with error " << err.what();
        RAW_LOG_ERROR("%s", msg->str().c_str());
        response.SetStatusCode(HttpStatusCode::CONFLICT);
        response.AddSize(msg->str().size());
        response.AddBody(std::move(msg));
        return;
    }

    response.SetStatusCode(HttpStatusCode::OK);
}

void HttpServer::Delete(const HttpRequest& request, HttpResponse& response) {
    if (request.HasHeader("Remove-Directory") && request.Header("Remove-Directory") == "True") {
        try {
            file_manager_.DeleteDirectory(request.Uri());
        } catch (std::runtime_error& err) {
            std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
            *msg << "Cannot delete directory " << request.Uri().c_str() << " with error " << err.what();
            RAW_LOG_ERROR("%s", msg->str().c_str());
            response.SetStatusCode(HttpStatusCode::CONFLICT);
            response.AddSize(msg->str().size());
            response.AddBody(std::move(msg));
            return;
        }

    } else {
        try {
            file_manager_.DeleteFile(request.Uri());
        } catch (std::runtime_error& err) {
            std::unique_ptr<std::stringstream> msg = std::make_unique<std::stringstream>();
            *msg << "Cannot delete file " << request.Uri().c_str() << " with error " << err.what();
            RAW_LOG_ERROR("%s", msg->str().c_str());
            response.SetStatusCode(HttpStatusCode::CONFLICT);
            response.AddSize(msg->str().size());
            response.AddBody(std::move(msg));
            return;
        }
    }
            
    response.SetStatusCode(HttpStatusCode::OK);
}
