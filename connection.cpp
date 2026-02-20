#include "connection.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/socket.h>

Connection::Connection(int fd)
    : fd_(fd), closed_(false) {}

Connection::~Connection() {
    if (!closed_) {
        close(fd_);
    }
}

int Connection::getFd() const {
    return fd_;
}

bool Connection::handleRead(std::string& out) {
    char buffer[1024];
    int n = recv(fd_, buffer, sizeof(buffer), 0);

    if (n <= 0) {
        closed_ = true;
        return false;
    }

    out.assign(buffer, n);
    std::cout << "Client says: " << out << std::endl;

    return true;
}

void Connection::sendMessage(const std::string& msg) {
    send(fd_, msg.c_str(), msg.size(), 0);
}

bool Connection::isClosed() const {
    return closed_;
}
