#pragma once

#include <cstdint>
#include <string>

class Listener {
public:
    Listener(std::string host, std::uint16_t port, int backlog_size = 10);
    ~Listener();

    Listener(Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

    int AcceptConnection();

private:
    void InitListener(std::string host, std::uint16_t port, int backlog_size);

    int fd_;
};