#include <cstdio>
#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <server.h>
#include <glog/raw_logging.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

Server::Server(std::string host, std::uint16_t port, 
    std::function<void(std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response)> process_request, 
    int backlog_size) 
    : listener_(host, port, backlog_size), process_request_(process_request) {
        if (process_request_ == nullptr) {
            process_request_ = [](std::unique_ptr<std::iostream>& request, std::list<std::unique_ptr<std::iostream>>& response) {
                std::unique_ptr<std::stringstream> msg;
                *msg << "Hello, world!\n";
                response.emplace_back(std::move(msg));
        };
    }

    RAW_LOG_INFO("Server created");
}

Server::~Server() {
    try {
        listener_th_.join();
        for (size_t i = 0; i < kWorkersCount; ++i) {
            workers_th_[i].join();
        }
    } catch (std::system_error& err) {
        RAW_LOG_ERROR("Thread join error in server dtor. %s", err.what());
    }

    RAW_LOG_INFO("Server closed");
}

void Server::Start() {
    listener_th_ = std::thread(&Server::ListenConnections, this);
    for (size_t i = 0; i < kWorkersCount; ++i) {
        workers_th_[i] = std::thread(&Server::HandleConnections, this, i);
    }

    RAW_LOG_INFO("Server started");
}

void Server::ListenConnections() {
    size_t current_worker = 0;
    while (true) {
        try {
            int connection_fd = listener_.AcceptConnection();
            epolls_[current_worker].AddFd(connection_fd);
            ++current_worker;
            current_worker %= kWorkersCount;
        } catch (std::runtime_error& err) {
            RAW_LOG_ERROR("%s", err.what());
        }
    }
}

void Server::HandleConnections(size_t worker_id) {
    while (true) {
        int nfds;

        try {
            nfds = epolls_[worker_id].Wait(events_[worker_id], kEventsCount);
        } catch (std::runtime_error& err) {
            RAW_LOG_ERROR("%s", err.what());
            continue;
        }

        for (int i = 0; i < nfds; ++ i) {
            if (events_[worker_id][i].events & EPOLLHUP) {
                Disconnect(events_[worker_id][i].data.fd);
                continue;
            }

            if (events_[worker_id][i].events & EPOLLIN) {
                HandleRequest(events_[worker_id][i].data.fd);
            }
        }
    }
}

void Server::HandleRequest(int fd) {
    std::unique_ptr<std::iostream> request = std::make_unique<std::stringstream>();

    try {
        ReadRequest(fd, *request);
    } catch (std::runtime_error& err) {
        RAW_LOG_ERROR("%s", err.what());
        Disconnect(fd);
        return;
    }

    std::list<std::unique_ptr<std::iostream>> response;

    RAW_LOG_INFO("Start process request on fd #%d", fd);

    process_request_(request, response);

    RAW_LOG_INFO("Request on fd #%d processed", fd);

    WriteResponse(fd, response);

    RAW_LOG_INFO("Write to fd #%d finished", fd);

    Disconnect(fd);
}

void Server::ReadRequest(int fd, std::iostream& request) {
    int retries = 100;
    char buffer[1024 * 1024];
    ssize_t count;
    while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
        request.write(buffer, count);
    }
}

void Server::WriteResponse(int fd, std::list<std::unique_ptr<std::iostream>>& response) {
    int retries = 100;
    RAW_LOG_INFO("%d", int(response.size()));
    for (auto& stream : response) {
        int buff_size = 1024 * 1024;
        char buffer[buff_size];
        
        while (stream->read(buffer, buff_size)) {
            int n = stream->gcount();
            RAW_LOG_INFO("Write %d bytes to buffer.", int(n));

            int total_bytes_writen = 0;
            while (total_bytes_writen != n) {
                int curr_bytes_writen = write(fd, &buffer[total_bytes_writen], n - total_bytes_writen);
                if (curr_bytes_writen == -1) {
                    if (errno == EAGAIN) {
                        sleep(3);
                        continue;
                    }
                    RAW_LOG_ERROR("Error: %s", std::strerror(errno));
                    break;
                }
                if (curr_bytes_writen == 0) {
                    RAW_LOG_INFO("Cannot write to socket %d/%d bytes. Try again.", n - total_bytes_writen, total_bytes_writen);
                    --retries;
                    if (retries < 0) {
                        RAW_LOG_ERROR("Error write to fd #%d.", fd);
                        return;
                    }
                    continue;
                }


                total_bytes_writen += curr_bytes_writen;
                RAW_LOG_INFO("Write %d bytes to socket.", total_bytes_writen);
            }

            if (total_bytes_writen <= 0) {
                int err = errno;
                RAW_LOG_ERROR("Error write to fd #%d. %s", fd, std::strerror(err));
            } 
        }

        if (stream->gcount() != 0) {
            int n = stream->gcount();
            RAW_LOG_INFO("Write %d bytes to buffer.", int(n));

            int total_bytes_writen = 0;
            while (total_bytes_writen != n) {
                int curr_bytes_writen = write(fd, &buffer[total_bytes_writen], n - total_bytes_writen);
                if (curr_bytes_writen == -1) {
                    if (errno == EAGAIN) {
                        sleep(3);
                        continue;
                    }
                    RAW_LOG_ERROR("Error: %s", std::strerror(errno));
                    break;
                }
                if (curr_bytes_writen == 0) {
                    RAW_LOG_INFO("Cannot write to socket %d/%d bytes. Try again.", n - total_bytes_writen, total_bytes_writen);
                    --retries;
                    if (retries < 0) {
                        RAW_LOG_ERROR("Error write to fd #%d.", fd);
                        return;
                    }
                    continue;
                }


                total_bytes_writen += curr_bytes_writen;
                RAW_LOG_INFO("Write %d bytes to socket.", total_bytes_writen);
            }

            if (total_bytes_writen <= 0) {
                int err = errno;
                RAW_LOG_ERROR("Error write to fd #%d. %s", fd, std::strerror(err));
            } 
        }
    }
}

void Server::Disconnect(int fd) {
    int res = close(fd);
    if (res < 0) {
        int err = errno;
        RAW_LOG_ERROR("Error close fd #%d after disconnect. %s", fd, std::strerror(err));
    }
    RAW_LOG_INFO("Close fd #%d", fd);
}



