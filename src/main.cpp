#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>

#include "comms/UdpClientSocket.h"
#include "comms/UdpServerSocket.h"

void commandMenu();
void definingRoute(std::vector<std::string>& commandsList);
void startRoute(const std::vector<std::string>& commandsList, UdpClient& droneClient);
void droneStatusCheck(UdpServer& serverSocket, std::atomic<bool>& keepRunning);

// communication with the drone 
// opening network channel w a socket

// the approach here is using no library so we're using POSIX and system headers

int main() {

    // INITIALIZING SEND COMMAND & RECEIVE RESPONSE PORT 8889
    UdpClient droneClient;

    int commsSocket = droneClient.initialSocket();

    if (commsSocket == -1) {
        return -1;
    }

    // ____________________________________________

    // INITIALIZING RECEIVE TELLO STATE PORT 8890
    UdpServer receivingServer;

    int serverSocket = receivingServer.initialSocket();

    if (serverSocket == -1) {
        return -1;
    }

    // BACKGROUND THREAD FOR STATUS MONITORING
    std::atomic<bool> isRunning(true);
    std::thread workerThread(droneStatusCheck, std::ref(receivingServer), std::ref(isRunning)); // std::ref(serverSocket)

    // ____________________________________________

    // DEFINING ROUTE AND STARTING IT

    std::vector<std::string> commandsList; 
   
    while(commandsList.size() == 0) {
        definingRoute(commandsList);
    }

    startRoute(commandsList, droneClient); 
    // ____________________________________________

    // TBA...

    // ____________________________________________

    // CLOSING CONNECTIONS
    // droneClient.closeCommsConnection(); // NOT NEEDED DESTRUCTOR DOES IT 

    // closing the background thread for status monitoring
    isRunning = false;
    workerThread.join();

    return 0;
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
        if (currentCommand == "0" && !commandsList.empty()) {
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
                    return;
                }   
            }
            
            // adding a command 
        } else if (!currentCommand.empty() && currentCommand != "0") {
            commandsList.push_back(currentCommand);
        }
    }

}

void startRoute(const std::vector<std::string>& commandsList, UdpClient& droneClient) {
    droneClient.sendCommand("takeoff");
    
    for (int i = 0; i < commandsList.size(); i++){
        
        droneClient.sendCommand(commandsList[i]);
        std::this_thread::sleep_for(std::chrono::seconds(1));  // added delay for the drone to process commands
    }

    droneClient.sendCommand("land");
}

void droneStatusCheck(UdpServer& serverSocket, std::atomic<bool>& keepRunning){
    std::ofstream logFile("drone_status_log.txt", std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        return;
    }

    while (keepRunning) {
        std::string message = serverSocket.listeningToStatus();


        // PRINT
        // std::cout << message << std::endl;

        // WRITE TO FILE
        logFile << message << std::endl;
    }

    logFile.close();
}