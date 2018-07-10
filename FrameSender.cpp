#include "FrameSender.h"
#include "ConcurrentDeque.h"
#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>
#include <unistd.h>

void sendMat(cv::Mat & mat, int fdPipe);

void threadCycle(ConcurrentDeque<cv::Mat> & cDeque, std::atomic_ulong & counter, int fdPipe, int order, int totalThreads) {
    std::chrono::duration<int, std::milli> sleepTime(50);
    while (true)
    {
        int tmp = counter.load() % totalThreads;
        if (tmp == order)
        {
            if (cDeque.empty())
                continue;
            cv::Mat frame = cDeque.pop_front();
            std::cout << order << " " << counter.load() << std::endl;
            sendMat(frame, fdPipe);
            counter++;
            if (frame.empty())
                break;
        } else
            std::this_thread::sleep_for(sleepTime);
    }
}

void sendAll(int fdPipe, char * buf, int len) {
    int total = 0;
    while (total < len) {
        int sent = write(fdPipe, buf, len - total);
        if (sent < 0) {
            perror("write");
            return;
        }
        total += sent;
    }
}


void sendMat(cv::Mat & mat, int fdPipe)
{
//    char * bytes = new char[size];  // you will have to delete[] that later
//    std::memcpy(bytes,frame.data,size * sizeof(char));


    sendAll(fdPipe, (char*)&mat.rows, sizeof(int));
    sendAll(fdPipe, (char*)&mat.cols, sizeof(int));
    int matType = mat.type();
    sendAll(fdPipe, (char*)&matType, sizeof(int));
    char * sBuf = (char *) mat.data;
    int matSize = mat.total() * mat.elemSize();
    sendAll(fdPipe, (char*)&matSize, sizeof(int));
    sendAll(fdPipe, sBuf, matSize);
}

FrameSender::FrameSender(ConcurrentDeque<cv::Mat> &cDeque, std::atomic_ulong &counter, int fdPipe, int order,
                         int totalThreads)
        : concurrentDeque(cDeque), counter(counter), fdPipe(fdPipe), order(order), totalThreads(totalThreads)
{

}

void FrameSender::start() {
    thread = new std::thread(threadCycle, std::ref(concurrentDeque), std::ref(counter), fdPipe, order, totalThreads);
}

void FrameSender::wait() {
    thread->join();
}

FrameSender::~FrameSender() {
    delete thread;
}
