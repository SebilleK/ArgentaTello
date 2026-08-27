// Receive Tello State UDP server
#pragma once


#include <string>
#include <vector>

class UdpServer {
    private:
        int currentSocket;
    public:
        UdpServer();
        ~UdpServer();
    
        int initialSocket();
        std::string listeningToStatus(); 
        void closeServer();
};