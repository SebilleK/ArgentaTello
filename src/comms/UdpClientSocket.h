// Send Command & Receive Response UDP Client
#pragma once

#include <string>
#include <vector>

class UdpClient {
    private:
        int currentSocket;

    public:
        UdpClient();
        ~UdpClient();

        int initialSocket();
        void sendCommand(std::vector<std::string>& command);
        void closeCommsConnection();
};