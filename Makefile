CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc
TARGET = argenta
SRCS = src/main.cpp src/comms/UdpClientSocket.cpp 

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)