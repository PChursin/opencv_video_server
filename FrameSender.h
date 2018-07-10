//
// Created by pavel on 08.07.18.
//

#ifndef OPENCV_VIDEO_SERVER_FRAMESENDER_H
#define OPENCV_VIDEO_SERVER_FRAMESENDER_H

#include "ConcurrentDeque.h"
#include <atomic>
#include "opencv2/opencv.hpp"
#include <thread>

class FrameSender {
public:
    FrameSender(ConcurrentDeque<cv::Mat> & cDeque, std::atomic_ulong & counter, int fdPipe, int order, int totalThreads);
    void start();
    //void threadCycle(ConcurrentDeque<cv::Mat> & cDeque, std::atomic_ulong & counter, int order, int totalThreads);
    void wait();

    ~FrameSender();

private:
    //void sendFrame(cv::Mat & frame);

    ConcurrentDeque<cv::Mat> & concurrentDeque;
    std::atomic_ulong & counter;
    int fdPipe;
    int order;
    int totalThreads;
    std::thread  * thread;
};


#endif //OPENCV_VIDEO_SERVER_FRAMESENDER_H
