// Receive Tello Video Stream UDP Server
// Uses OpenCV

#include "StreamReceiver.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <filesystem>

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

static const std::vector<std::string> COCO_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

// runs open cv video loop
void UdpStream::listeningToStream(std::atomic<bool>& keepRunning){
    setenv("OPENCV_FFMPEG_LOGLEVEL", "0", 1); // muting FFmpeg output

    // MODEL LOADING ___________________________________________________

    cv::dnn::Net net;
    // std::cout << std::filesystem::current_path();
    try {
        const char* modelPath = std::getenv("MODEL_PATH");
        net = cv::dnn::readNetFromONNX(modelPath ? modelPath : "model/yolov8n.onnx");

    } catch (const cv::Exception& e) {
        std::cerr << "Failed to load ONNX model: " << e.what() << std::endl;
        return;
    }

    // CPU execution
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    const float CONFIDENCE_THRESHOLD = 0.45f;
    const float NMS_THRESHOLD = 0.50f;
    const cv::Size MODEL_INPUT_SIZE(640, 640);

    cv::Mat frame;
    
    // ______________________________________________________________________

    // std::cout << "UDP Stream Server listening on port 11111...\n" << std::endl;
    cv::VideoCapture cap("udp://@0.0.0.0:11111?overrun_nonfatal=1&fifo_size=50000", cv::CAP_FFMPEG);

    if (!cap.isOpened()) {
        std::cerr << "Failed to open Tello Video Stream on port 11111" << std::endl;
        return;
    }

    // cv::Mat frame;

    // window resizing 
    cv::namedWindow("Tello Video Stream", cv::WINDOW_GUI_NORMAL);
    cv::resizeWindow("Tello Video Stream", 1920, 1080);

    while (keepRunning) {
        cap >> frame;
        if (frame.empty()) {
            continue;
        }

        // preprocessing 
        cv::Mat blob;
        cv::dnn::blobFromImage(
            frame,
            blob,
            1.0 / 255.0,        // Normalize [0, 255] to [0.0, 1.0]
            MODEL_INPUT_SIZE,   // YOLOv8 input resolution
            cv::Scalar(),       // No mean subtraction
            true,               // Swap BGR to RGB
            false               // No center crop
        );

        net.setInput(blob);

        // propragation
        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        // parsing output
        cv::Mat rawOutput = outputs[0];
        cv::Mat reshapedOutput = rawOutput.reshape(1, rawOutput.size[1]);
        cv::Mat transposedOutput;
        cv::transpose(reshapedOutput, transposedOutput);

        float* data = reinterpret_cast<float*>(transposedOutput.data);
        int dimensions = transposedOutput.cols; 
        int rows = transposedOutput.rows;

        float xFactor = static_cast<float>(frame.cols) / MODEL_INPUT_SIZE.width;
        float yFactor = static_cast<float>(frame.rows) / MODEL_INPUT_SIZE.height;

        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        for (int i = 0; i < rows; ++i) {
            float* rowPtr = data + (i * dimensions);
            float* classScores = rowPtr + 4;

            cv::Mat scores(1, static_cast<int>(COCO_CLASSES.size()), CV_32FC1, classScores);
            cv::Point classIdPoint;
            double maxClassScore = 0.0;
            cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);

            if (maxClassScore > CONFIDENCE_THRESHOLD) {
                float cx = rowPtr[0];
                float cy = rowPtr[1];
                float w  = rowPtr[2];
                float h  = rowPtr[3];

                int left   = static_cast<int>((cx - 0.5f * w) * xFactor);
                int top    = static_cast<int>((cy - 0.5f * h) * yFactor);
                int width  = static_cast<int>(w * xFactor);
                int height = static_cast<int>(h * yFactor);

                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(static_cast<float>(maxClassScore));
                classIds.push_back(classIdPoint.x);
            }
        }

        // filtering
        std::vector<int> nmsIndices;
        cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, nmsIndices);

        // drawing boxes
        for (int idx : nmsIndices) {
            cv::Rect box = boxes[idx] & cv::Rect(0, 0, frame.cols, frame.rows);
            int classId = classIds[idx];

            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);

            std::string label = COCO_CLASSES[classId] + ": " + cv::format("%.2f", confidences[idx]);

            int baseline = 0;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            int topText = std::max(box.y, labelSize.height);

            cv::rectangle(
                frame,
                cv::Point(box.x, topText - labelSize.height),
                cv::Point(box.x + labelSize.width, topText + baseline),
                cv::Scalar(0, 255, 0),
                cv::FILLED
            );

            cv::putText(
                frame,
                label,
                cv::Point(box.x, topText),
                cv::FONT_HERSHEY_SIMPLEX,
                0.5,
                cv::Scalar(0, 0, 0),
                1
            );
        }

        // _____________________________________________________________
        
        // _____________________________________________________________
        

        cv::imshow("Tello Video Stream", frame);

        // cv::waitKey(1) is required to render the window frames
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
}
