// MOCK SERVER / DRONE
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <thread>
#include <chrono>

int sendAndReceive();
int droneStatus();

int main() {
    // BG THREAD FOR DRONE STATUS
    std::thread statusThread(droneStatus); 
    std::cout << "UDP Mock Client sending info on port 8890... \n" << std::endl;

    // SEND & RECEIVE COMMANDS
    sendAndReceive();

    // JOIN
    if (statusThread.joinable()) {
        statusThread.join();
    }

    return 0;
}

int sendAndReceive() {
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

int droneStatus() {
    int client = socket(AF_INET, SOCK_DGRAM, 0);
        
    if (client < 0) {
        std::cerr << "Error creating client" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8890); // port number
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // inet_addr("192.168.10.1"); 

    connect(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    
    const char* initialMessage = "Data string received.... x y z blah blah"; 

    while (true) {
        send(client, initialMessage, strlen(initialMessage), 0);
        
        // 1 sec
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(client);
    return 0;
}