#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <deque>
#include <atomic>
#include "FrameSender.h"
#include "ConcurrentDeque.h"

#define THREADS_NUM 2
#define FRAME_LIMIT 150

using namespace cv;

int main(){


    VideoCapture cap(0);
    ConcurrentDeque<Mat> cDeque;
    std::atomic_ulong counter(0);
    FrameSender oddWriter(cDeque, counter, 0, THREADS_NUM);
    FrameSender evenWriter(cDeque, counter, 1, THREADS_NUM);

    if(!cap.isOpened()){
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    oddWriter.start();
    evenWriter.start();

    while(true){
        Mat frame;
        cap >> frame;
        if (frame.empty() || counter.load() >= FRAME_LIMIT)
        {
            for (int i = 0; i < THREADS_NUM; i++)
                cDeque.push_back(Mat());
            break;
        }

        cDeque.push_back(frame);

        //imshow( "Frame", frame );

        // Press  ESC on keyboard to exit
        /*auto c = (char)waitKey(25);
        if(c == 27)
        {
            for (int i = 0; i < THREADS_NUM; i++)
                cDeque.push_back(Mat());
            break;
        }*/

    }
    oddWriter.wait();
    evenWriter.wait();
    cap.release();
    destroyAllWindows();
    return 0;
}