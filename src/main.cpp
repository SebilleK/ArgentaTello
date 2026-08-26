#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "comms/UdpClientSocket.h"

void commandMenu();
void definingRoute(std::vector<std::string>& commandsList);
void startRoute(std::vector<std::string>& commandsList);

// communication with the drone 
// opening network channel w a socket

// the approach here is using no library so we're using POSIX and system headers

int main() {

    // INITIALIZING SEND COMMAND & RECEIVE RESPONSE
    UdpClient droneClient;

    int commsSocket = droneClient.initialSocket();

    if (commsSocket == -1) {
        return -1;
    }
    // ____________________________________________

    // DEFINING ROUTE AND STARTING IT

    std::vector<std::string> commandsList; 
   
    definingRoute(commandsList);
    startRoute(commandsList); 
    // ____________________________________________

    // TBA...

    // ____________________________________________

    // CLOSING CONNECTIONS
    droneClient.closeCommsConnection();

}


// Lookup table for inverted commands for return path
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
    std::cout << "_____________________ TELLO COMMANDS _____________________\n";
    std::cout << "  up x      | Ascend to \"x\" cm (x = 20-500)\n";
    std::cout << "  down x    | Descend to \"x\" cm (x = 20-500)\n";
    std::cout << "  left x    | Fly left for \"x\" cm (x = 20-500)\n";
    std::cout << "  right x   | Fly right \"x\" cm (x = 20-500)\n";
    std::cout << "  forward x | Fly forward for \"x\" cm (x = 20-500)\n";
    std::cout << "  back x    | Fly backward for \"x\" cm (x = 20-500)\n";
    std::cout << "  cw x      | Rotate \"x\" degrees clockwise (x = 1-360)\n";
    std::cout << "  ccw x     | Rotate \"x\" degrees counter-clockwise (x = 1-360)\n";
    std::cout << "  0         | Stop and execute\n";
    std::cout << "____________________________________________________________\n";
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

void startRoute(std::vector<std::string>& commandsList) {

    // communicate with the drone for path....
    // USES SEND COMMAND!!

    // TESTING
    for (int i = 0; i < commandsList.size(); i++){

        std::cout << commandsList[i] << "\n";
    }

}