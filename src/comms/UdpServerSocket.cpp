#include "UdpServerSocket.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

UdpServer::UdpServer() : currentSocket(-1) {}

UdpServer::~UdpServer() {
    closeServer();
}

int UdpServer::initialSocket(){
    // UDP SOCKET
    currentSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (currentSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // PORT REUSE
    int opt = 1;
    if (setsockopt(currentSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        closeServer();
        return -1;
    }


    // ADDRESS SETUP AND BIND
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0.0.0.0
    address.sin_port = htons(8890);

    if (bind(currentSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        closeServer();
        return -1;
    }

    std::cout << "UDP Server ready..." << std::endl;

    return currentSocket;
}

std::string UdpServer::listeningToStatus(){
    // std::cout << "UDP Server listening on port 8890...\n" << std::endl;

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    
    std::memset(buffer, 0, sizeof(buffer));
        
    ssize_t bytes_received = recvfrom(
        currentSocket, 
        buffer, 
        sizeof(buffer) - 1, 
        0, 
        (struct sockaddr*)&client_addr, 
        &client_len
        );

    if (bytes_received < 0) {
        std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
        return "";
    }

    buffer[bytes_received] = '\0';
    // PRINTING
    // std::cout << "Received status: " << buffer << std::endl; 
    return buffer;
}

void UdpServer::closeServer(){
    if (currentSocket >= 0) {
        close(currentSocket);
        currentSocket = -1;
    }
}