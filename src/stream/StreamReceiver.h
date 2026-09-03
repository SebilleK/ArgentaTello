// Receive Tello Video Stream UDP server
#pragma once

#include <atomic>

class UdpStream {
    public:
        void listeningToStream(std::atomic<bool>& keepRunning); 
};