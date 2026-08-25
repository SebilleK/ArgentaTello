#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>



// communication with the drone 
// opening network channel w a socket

// the approach here is using no library so we're using POSIX and system headers

int main() {


    // creating socket
    // https://www.linuxhowtos.org/manpages/2/socket.html
    int TextCommsClientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (TextCommsClientSocket < 0) {
        std::cerr << "Error creating TextCommsClientSocket" << std::endl;
        return 1;
    }

    
    std::cout << "UDP client set up successfully \n" << std::endl;

    // https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

    // server side socket

    // address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8889); // port number
    serverAddress.sin_addr.s_addr = inet_addr("192.168.10.1"); // SEND COMMAND & RECEIVE RESPONSE FROM DRONE


    // connecting to the server/drone for the first time
    connect(TextCommsClientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::cout << "Connected to the Tello drone successfully \n" << std::endl;

    const char* initialMessage = "command"; // initial command to enter SDK mode 

    send(TextCommsClientSocket, initialMessage, strlen(initialMessage), 0);

    // waiting for the drone response
    char bufferResponse[1024];
    ssize_t response = recv(TextCommsClientSocket, bufferResponse, sizeof(bufferResponse) - 1, 0);

    if (response > 0) {
        bufferResponse[response] = '\0'; // setting bit count to null char / stopping reading where received packet data ends
        std::cout << "Tello Drone:" << bufferResponse << std::endl;
    
    
        std::cout << "Sent initial command successfully \n Tello SDK mode initiated" << std::endl;
    } else { // -1 
        std::cout << "Failed to send initial command \n Aborting." << std::endl;
        close(TextCommsClientSocket);
    }

    // TBA 
    /// COMANDOS ....
    // 1. takeoff
    // more controls (premade route)
    // scanning as it goes
    // return and land

    
    close(TextCommsClientSocket);




    





    

    // 



}