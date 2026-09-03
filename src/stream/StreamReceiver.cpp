// Receive Tello Video Stream UDP Server
// Uses OpenCV

#include "StreamReceiver.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <opencv2/opencv.hpp>

// runs open cv video loop
void UdpStream::listeningToStream(std::atomic<bool>& keepRunning){
    // std::cout << "UDP Stream Server listening on port 11111...\n" << std::endl;
    cv::VideoCapture cap("udp://@0.0.0.0:11111");

    if (!cap.isOpened()) {
        std::cerr << "Failed to open Tello Video Stream on port 11111" << std::endl;
        return;
    }

    cv::Mat frame;

    while (keepRunning) {
        cap >> frame;
        if (frame.empty()) {
            continue;
        }

        cv::imshow("Tello Video Stream", frame);

        // cv::waitKey(1) is required to render the window frames
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
}
