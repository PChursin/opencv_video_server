#include "FrameSender.h"
#include "ConcurrentDeque.h"
#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>

void threadCycle(ConcurrentDeque<cv::Mat> & cDeque, std::atomic_ulong & counter, int order, int totalThreads) {
    std::chrono::duration<int, std::milli> sleepTime(50);
    while (true)
    {
        int tmp = counter.load() % totalThreads;
        if (tmp == order)
        {
            if (cDeque.empty())
                continue;
            cv::Mat frame = cDeque.pop_front();
            counter++;
            if (frame.empty())
                break;
            std::cout << order << " " << counter.load() << std::endl;
            //cv::imshow("Frame", frame);
        } else
            std::this_thread::sleep_for(sleepTime);
    }
}

FrameSender::FrameSender(ConcurrentDeque<cv::Mat> &cDeque, std::atomic_ulong &counter, int order, int totalThreads)
        : concurrentDeque(cDeque), counter(counter), order(order), totalThreads(totalThreads)
{

}

void FrameSender::start() {
    thread = new std::thread(threadCycle, std::ref(concurrentDeque), std::ref(counter), order, totalThreads);
}

void FrameSender::wait() {
    thread->join();
}

FrameSender::~FrameSender() {
    delete thread;
}
