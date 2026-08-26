// MOCK SERVER / DRONE
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

int main() {
    // UDP SOCKET
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // PORT REUSE
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // ADDRESS SETUP AND BIND
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8889);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "UDP Mock Server listening on port 8889..." << std::endl;

    // RECEIVE/RESPOND LOOP
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        
        ssize_t bytes_received = recvfrom(
            server_fd, 
            buffer, 
            sizeof(buffer) - 1, 
            0, 
            (struct sockaddr*)&client_addr, 
            &client_len
        );

        if (bytes_received < 0) {
            std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Received command: " << buffer << std::endl;

        // DRONE RESPONSE
        const char* response = "ok";
        sendto(
            server_fd, 
            response, 
            std::strlen(response), 
            0, 
            (struct sockaddr*)&client_addr, 
            client_len
        );
    }

    close(server_fd);
    return 0;
}