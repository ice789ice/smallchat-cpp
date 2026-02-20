#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>
#include <unordered_map>
#include "connection.h"

class Server {
public:
    explicit Server(int port);
    void start();

private:
    void handleNewConnection();
    void handleClientMessage(int fd);

    int listenFd_;
    struct ClientState {
        std::shared_ptr<Connection> conn;
        bool has_nickname;
        std::string nickname;
    };
    std::unordered_map<int, ClientState> connections_;
};

#endif
