#pragma once

#include <sys/epoll.h>

class Epoll {
public:
    Epoll();
    ~Epoll();

    int Wait(epoll_event* events, size_t events_count);

    void AddFd(int fd);

private:
    int fd_;
};