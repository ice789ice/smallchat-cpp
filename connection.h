#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;
    bool handleRead(std::string& out);
    void sendMessage(const std::string& msg);
    bool isClosed() const;

private:
    int fd_;
    bool closed_;
};

#endif
