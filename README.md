# WIP


## Makefile

Make sure you're in the root directory.

**Build and run**
```bash
make
./argenta
```

**Remove**
```bash
make clean
```

### Manual

Syntax check
```bash
g++ -fsyntax-only -std=c++17 -Wall -Wextra -Isrc [filename_path]
```
**ex: src/main.cpp or src/comms/UdpClientSocket.cpp**

Creating executable
```bash
g++ -std=c++17 -Wall -Wextra src/main.cpp src/comms/UdpClientSocket.cpp -Isrc -o argenta
```
**use all files**


## Mock Server/Client

Local mock server for testing command logic without the drone (PORT 8889). 
Also sends fake status updates and writes them continously to a drone_status_log.txt file (PORT 8890).
**Change client address to 127.0.0.1**.

Creating executable and running
```bash
g++ TestServer.cpp -o server
./server
```

## Tello Official Docs

**SDK:**
https://dl-cdn.ryzerobotics.com/downloads/Tello/Tello%20SDK%202.0%20User%20Guide.pdf

**USER GUIDE:**
https://dl-cdn.ryzerobotics.com/downloads/Tello/20180212/Tello+User+Manual+v1.0_EN_2.12.pdf
