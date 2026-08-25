#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <list>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>


void commandMenu();
void definingRoute(std::vector<std::string>& commandsList);
void startRoute(std::vector<std::string>& commandsList);

// communication with the drone 
// opening network channel w a socket

// the approach here is using no library so we're using POSIX and system headers

int main() {


    //! CREATING SOCKET, INITIAL COMMUNICATION
    //________________________________________
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

    std::cout << "Attempting to connect to the Tello drone...\nTimeout: 5 secs" << std::endl;
   

    const char* initialMessage = "command"; // initial command to enter SDK mode 

    send(TextCommsClientSocket, initialMessage, strlen(initialMessage), 0);

   
    // timer to wait for response
    struct timeval responseTimeout;
    responseTimeout.tv_sec = 5;
    responseTimeout.tv_usec = 0;

    // setting timout
    setsockopt(TextCommsClientSocket, SOL_SOCKET, SO_RCVTIMEO, &responseTimeout, sizeof(responseTimeout));


    // waiting for the drone response
    char bufferResponse[1024];
    ssize_t response = recv(TextCommsClientSocket, bufferResponse, sizeof(bufferResponse) - 1, 0);


    // TIMER 
    if (response < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "Timeout. Drone didn't respond within 5 secs. \n";
        } else {
            std::cout << "Failed to send initial command \n Aborting." << std::endl;
            std::cerr << "Socket read error: " << strerror(errno) << " (code: " << errno << ")" << std::endl;
        }

        close(TextCommsClientSocket);
        return 1;
    }
  
    std::cout << "Received drone response. \n" << std::endl;

    if (response > 0) {
        bufferResponse[response] = '\0'; // setting bit count to null char / stopping reading where received packet data ends
        std::cout << "Tello Drone:" << bufferResponse << std::endl;
    
    
        std::cout << "Sent initial command successfully \n Tello SDK mode initiated" << std::endl;
    } 

    //________________________________________

    std::vector<std::string> commandsList; 
   
    // DEFINING ROUTE AND STARTING IT
    definingRoute(commandsList);
    startRoute(commandsList);

    // ________________________________________

    // LATER: VIDEO STREAM
    // SCANNING + ENVIAR ALERTS

    // ________________________________________
    close(TextCommsClientSocket);

}


// Lookup table for inverted commands
const std::unordered_map<std::string, std::string> opposites = {
    {"up", "down"},
    {"down", "up"},
    {"left", "right"},
    {"right", "left"},
    {"forward", "back"},
    {"back", "forward"},
    {"cw", "ccw"},
    {"ccw", "cw"}
};


void commandMenu() {
    std::cout << "_____________________ TELLO COMMANDS _____________________";
    std::cout << "  up x      | Ascend to \"x\" cm (x = 20-500)\n";
    std::cout << "  down x    | Descend to \"x\" cm (x = 20-500)\n";
    std::cout << "  left x    | Fly left for \"x\" cm (x = 20-500)\n";
    std::cout << "  right x   | Fly right \"x\" cm (x = 20-500)\n";
    std::cout << "  forward x | Fly forward for \"x\" cm (x = 20-500)\n";
    std::cout << "  back x    | Fly backward for \"x\" cm (x = 20-500)\n";
    std::cout << "  cw x      | Rotate \"x\" degrees clockwise (x = 1-360)\n";
    std::cout << "  ccw x     | Rotate \"x\" degrees counter-clockwise (x = 1-360)\n";
    std::cout << "  0         | Stop and execute\n";
    std::cout << "____________________________________________________________";
}


void definingRoute(std::vector<std::string>& commandsList) {
    bool continueInput = true;
        

    commandMenu();
    while(continueInput) {
        std::string currentCommand;
        std::cout << "Please input the next control command for the drone" << std::endl;
                    
        std::cout << "Command: " << std::endl;

        std::getline(std::cin, currentCommand);


        // stopping adding commands and defining return path w/inverse commands
        if (currentCommand == "0") {
            continueInput = false; 

            size_t originalSize = commandsList.size();
            // size_t 
            for (int i = originalSize - 1; i >= 0; i--){
                std::string returnCommand;

                std::stringstream ss(commandsList[i]); // reading string and parsing
                std::string action, param;
                ss >> action >> param;
                    
                // auto is iterator
                auto it = opposites.find(action);

                // if the command is found
                if (it != opposites.end()) {
                    // it->first ; it->second key ; value
                    returnCommand = it->second + " " + param;
                    commandsList.push_back(returnCommand);
                } else {
                    std::cerr << "It was not possible to define a return route.\n Please try to input a path again." << std::endl;
                    commandsList.clear();
                    definingRoute(commandsList);
                    return;
                }   
            }
            
            // adding a command 
        } else if (!currentCommand.empty()) {
            commandsList.push_back(currentCommand);
        }
    }

}

// SHOULD RECEIVE DRONE CURRENT STATE
void startRoute(std::vector<std::string>& commandsList) {

    // communicate with the drone for path....

    // TESTING
    for (int i = 0; i < commandsList.size(); i++){

        std::cout << commandsList[i] << "\n";
    }

}