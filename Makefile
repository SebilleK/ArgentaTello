CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc $(shell pkg-config --cflags opencv4)
LDFLAGS = $(shell pkg-config --libs opencv4)
TARGET = argenta
SRCS = src/main.cpp src/comms/UdpClientSocket.cpp src/comms/UdpServerSocket.cpp src/stream/StreamReceiver.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)