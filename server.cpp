#include "server.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

Server::Server(int port) : listenFd_(-1) {
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        std::cerr << "socket failed: " << std::strerror(errno) << std::endl;
        return;
    }

    int opt = 1;
    if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << std::strerror(errno) << std::endl;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind failed: " << std::strerror(errno) << std::endl;
        close(listenFd_);
        listenFd_ = -1;
        return;
    }

    if (listen(listenFd_, 16) < 0) {
        std::cerr << "listen failed: " << std::strerror(errno) << std::endl;
        close(listenFd_);
        listenFd_ = -1;
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;
}

void Server::start() {
    if (listenFd_ < 0) {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listenFd_, &readfds);
        int maxFd = listenFd_;

        for (const auto& it : connections_) {
            FD_SET(it.first, &readfds);
            if (it.first > maxFd) {
                maxFd = it.first;
            }
        }

        int ready = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "select failed: " << std::strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(listenFd_, &readfds)) {
            handleNewConnection();
        }

        std::vector<int> toRemove;
        for (auto& it : connections_) {
            int fd = it.first;
            if (!FD_ISSET(fd, &readfds)) {
                continue;
            }

            std::string msg;
            if (!it.second.conn->handleRead(msg) || it.second.conn->isClosed()) {
                toRemove.push_back(fd);
                continue;
            }

            while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
                msg.pop_back();
            }

            if (!it.second.has_nickname) {
                it.second.nickname = msg;
                it.second.has_nickname = true;
                std::cout << "fd " << fd << " nickname = " << it.second.nickname << std::endl;
                continue;
            }

            if (msg == "/quit") {
                std::string left = "[system]: " + it.second.nickname + " left\n";
                for (auto& dst : connections_) {
                    if (dst.first != fd && dst.second.has_nickname) {
                        dst.second.conn->sendMessage(left);
                    }
                }
                toRemove.push_back(fd);
                continue;
            }

            if (!msg.empty()) {
                std::string out = "[" + it.second.nickname + "]: " + msg + "\n";
                for (auto& dst : connections_) {
                    if (dst.first != fd && dst.second.has_nickname) {
                        dst.second.conn->sendMessage(out);
                    }
                }
            }
        }

        for (int fd : toRemove) {
            connections_.erase(fd);
            close(fd);
        }
    }
}

void Server::handleNewConnection() {
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(listenFd_, reinterpret_cast<sockaddr*>(&clientAddr), &len);
    if (clientFd < 0) {
        std::cerr << "accept failed: " << std::strerror(errno) << std::endl;
        return;
    }

    connections_[clientFd] = {std::make_shared<Connection>(clientFd), false, ""};
    std::cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr)
              << ":" << ntohs(clientAddr.sin_port) << std::endl;
}

void Server::handleClientMessage(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return;
    }
    std::string msg;
    it->second.conn->handleRead(msg);
}
