#pragma once

#include <epoll.h>
#include <listener.h>

#include <list>
#include <cstdint>
#include <istream>
#include <functional>
#include <string>
#include <thread>

class Server {
public:
    explicit Server(std::string host, std::uint16_t port,
    std::function<void(std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response)> process_request = nullptr, 
    int backlog_size = 10);

    Server(Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&) = delete;
    Server& operator=(Server&&) = delete;

    ~Server();

    void Start();

    void ListenConnections();

    void HandleConnections(size_t worker_id);

    void HandleRequest(int fd);

    void Disconnect(int fd);


private:
    void WriteResponse(int fd, std::list<std::unique_ptr<std::iostream>>& response);
    void ReadRequest(int fd, std::iostream& request);

    static const std::size_t kWorkersCount = 10;
    static const std::size_t kEventsCount = 10;

    std::thread listener_th_;
    Listener listener_;

    std::thread workers_th_[kWorkersCount];
    Epoll epolls_[kWorkersCount];
    epoll_event events_[kWorkersCount][kEventsCount];

    std::function<void(std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response)> process_request_;
};