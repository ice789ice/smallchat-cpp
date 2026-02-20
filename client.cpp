#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

int main() {
    const char* server_ip = "127.0.0.1";
    int server_port = 8888;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "socket failed: " << std::strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(static_cast<uint16_t>(server_port));
    if (inet_pton(AF_INET, server_ip, &serv.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << server_ip << std::endl;
        return 1;
    }

    if (connect(sockfd, reinterpret_cast<sockaddr*>(&serv), sizeof(serv)) < 0) {
        std::cerr << "connect failed: " << std::strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Welcome to SmallChat!\n";
    std::cout << "Please enter your nickname and press Enter:\n";

    fd_set readfds;
    std::string buf(4096, '\0');
    bool nickname_sent = false;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);

        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
        int ret = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "select failed: " << std::strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, buf.data(), buf.size());
            if (n > 0) {
                if (send(sockfd, buf.data(), static_cast<size_t>(n), 0) < 0) {
                    std::cerr << "send failed: " << std::strerror(errno) << std::endl;
                    break;
                }
                if (!nickname_sent) {
                    nickname_sent = true;
                    std::cout << "You can start chatting now.\n";
                    std::cout << "Type /quit to exit.\n";
                }
            }
        }

        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t n = recv(sockfd, buf.data(), buf.size(), 0);
            if (n <= 0) {
                break;
            }
            std::cout.write(buf.data(), n);
            std::cout.flush();
        }
    }

    close(sockfd);
    return 0;
}
