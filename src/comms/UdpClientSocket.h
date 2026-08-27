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
        int sendCommand(const std::string& command);
        int receiveCommandResponse();
        void closeCommsConnection();
};