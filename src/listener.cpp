#include <arpa/inet.h>
#include <listener.h>
#include <glog/raw_logging.h>

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <arpa/inet.h>

Listener::Listener(std::string host,std::uint16_t port, int backlog_size) {
    try {
        InitListener(host, port, backlog_size);
        std::ostringstream msg;
        RAW_LOG(INFO, "Listener is initialized on address: %s:%d", host.c_str(), port);
    } catch (std::runtime_error& err) {
        RAW_LOG(FATAL, "Listener initialization error. %s", err.what());
    }
}

Listener::~Listener() {
    if (fd_ != -1) {
        close(fd_);
    }

    RAW_LOG(INFO, "Listener is closed");
}

int Listener::AcceptConnection() {
    epoll_event event;

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(fd_, (sockaddr*)&client_addr, &client_len);

    if (client_fd == -1) {
        int err = errno;
        std::ostringstream msg;
        msg << "Listener did not accept connection from " << client_addr.sin_addr.s_addr << ":" << client_addr.sin_port 
        << " Error: " << std::strerror(err);
        throw std::runtime_error(msg.str());
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), addr, INET_ADDRSTRLEN);
    RAW_LOG(INFO, "Listener accepted connection from %s:%d on fd %d", addr, client_addr.sin_port, client_fd);
    return client_fd;
}

void Listener::InitListener(std::string host, std::uint16_t port, int backlog_size) {
    if ( (fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        int err = errno;
        std::ostringstream msg;
        msg << "Failed to create a TCP socket. " << std::strerror(err);
        throw std::runtime_error(msg.str());
    }

    int opt = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
            &opt, sizeof(opt)) < 0) {
        int err = errno;
        std::ostringstream msg;
        msg << "Failed to set socket options. " << std::strerror(err);
        throw std::runtime_error(msg.str());
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &(server_address.sin_addr));

    if (bind(fd_, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        int err = errno;
        std::ostringstream msg;
        msg << "Failed to bind to socket. " << std::strerror(err);
        std::runtime_error(msg.str());
    }

    if (listen(fd_, backlog_size) < 0) {
        int err = errno;
        std::ostringstream msg;
        msg << "Failed to listen on port: " << port << ". " << std::strerror(err);
        std::runtime_error(msg.str());
    }
}