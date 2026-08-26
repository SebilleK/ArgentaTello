#include "UdpClientSocket.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

int currentSocket;

UdpClient::UdpClient() : currentSocket(-1) {}

UdpClient::~UdpClient() {
    closeCommsConnection();
}

int UdpClient::initialSocket(){
    //! CREATING SOCKET, INITIAL COMMUNICATION
    //________________________________________
    // https://www.linuxhowtos.org/manpages/2/socket.html
    currentSocket = socket(AF_INET, SOCK_DGRAM, 0);
        

    if (currentSocket < 0) {
        std::cerr << "Error creating currentSocket" << std::endl;
        return -1;
    }

        
    std::cout << "UDP client set up successfully \n" << std::endl;

    // https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

    // server side socket

    // address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8889); // port number
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // inet_addr("192.168.10.1"); // SEND COMMAND & RECEIVE RESPONSE FROM DRONE


    // connecting to the server/drone for the first time
    connect(currentSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::cout << "Attempting to connect to the Tello drone...\nTimeout: 5 secs" << std::endl;
    

    const char* initialMessage = "command"; // initial command to enter SDK mode 

    send(currentSocket, initialMessage, strlen(initialMessage), 0);

    
    // timer to wait for response
    struct timeval responseTimeout;
    responseTimeout.tv_sec = 5;
    responseTimeout.tv_usec = 0;

    // setting timout
    setsockopt(currentSocket, SOL_SOCKET, SO_RCVTIMEO, &responseTimeout, sizeof(responseTimeout));


    // waiting for the drone response
    char bufferResponse[1024];
    ssize_t response = recv(currentSocket, bufferResponse, sizeof(bufferResponse) - 1, 0);


    // TIMER 
    if (response <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "Timeout. Drone didn't respond within 5 secs. \n";
        } else {
            std::cout << "Failed to send initial command \n Aborting." << std::endl;
            std::cerr << "Socket read error: " << strerror(errno) << " (code: " << errno << ")" << std::endl;
        }
        close(currentSocket);
        return -1;
    }
    
    std::cout << "Received drone response. \n" << std::endl;

    if (response > 0) {
        bufferResponse[response] = '\0'; // setting bit count to null char / stopping reading where received packet data ends
        std::cout << "Tello Drone:" << bufferResponse << std::endl;
    
    
        std::cout << "Sent initial command successfully \nTello SDK mode initiated" << std::endl;
        return currentSocket;
    } 

}
        
int UdpClient::sendCommand(const std::string& command){

    send(currentSocket, command.data(), command.size(), 0); // .data() => raw pointer
     
    // timer to wait for response
    struct timeval responseTimeout;
    responseTimeout.tv_sec = 5;
    responseTimeout.tv_usec = 0;

    // setting timout
    setsockopt(currentSocket, SOL_SOCKET, SO_RCVTIMEO, &responseTimeout, sizeof(responseTimeout));


    // waiting for the drone response
    char bufferResponse[1024];
    ssize_t response = recv(currentSocket, bufferResponse, sizeof(bufferResponse) - 1, 0);


    // TIMER 
    if (response <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "Timeout. Drone didn't respond within 5 secs. \n";
        } else {
            std::cout << "Failed to send command \n Aborting." << std::endl;
            std::cerr << "Socket read error: " << strerror(errno) << " (code: " << errno << ")" << std::endl;
        }
        close(currentSocket);
        return -1;
    }
    

    if (response > 0) {
        bufferResponse[response] = '\0'; // setting bit count to null char / stopping reading where received packet data ends
        std::cout << "Tello Drone:" << bufferResponse << std::endl;
    
    
        std::cout << "Sent command successfully" << std::endl;
        return currentSocket;
    } 

}

void UdpClient::closeCommsConnection() { 
    if (currentSocket >= 0) {
        close(currentSocket);
        currentSocket = -1;
    }
}
