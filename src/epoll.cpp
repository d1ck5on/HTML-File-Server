#include <cstring>
#include <epoll.h>
#include <glog/raw_logging.h>

#include <cerrno>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <sstream>

Epoll::Epoll() : fd_(epoll_create1(0)) {
    if (fd_ < 0) {
        int err = errno;
        RAW_LOG_FATAL("Epoll initialization failed with error %s", std::strerror(err));
    }
    RAW_LOG_INFO("Epoll fd #%d created", fd_);
};

Epoll::~Epoll() {
    if (fd_ != -1) {
        close(fd_);
    }
    RAW_LOG_INFO("Epoll is closed");
}

int Epoll::Wait(epoll_event* events, size_t events_count) {
    int fd = epoll_wait(fd_, events, events_count, -1);

    if (fd < 0) {
        std::ostringstream msg;
        int err = errno;
        msg << "Epoll wait error. " << std::strerror(err);
        throw std::runtime_error(msg.str());
    }

    return fd;
}

void Epoll::AddFd(int fd) {
    epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLHUP;
    event.data.fd = fd;
    if ( (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event)) < 0) {
        std::ostringstream msg;
        int err = errno;
        msg << "Error adding fd to epoll. " << std::strerror(err);
        throw std::runtime_error(msg.str());
    }

    RAW_LOG_INFO("Added fd #%d to epoll", fd);
}

