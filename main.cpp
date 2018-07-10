#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <deque>
#include <atomic>
#include "FrameSender.h"
#include "ConcurrentDeque.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define THREADS_NUM 2
#define FRAME_LIMIT 1500
#define FIFO_NAME "/tmp/opencv_video"

using namespace cv;

void readAll(int fdPipe, char * buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int bytes = read(fdPipe, buf+total, len);
        if (bytes < 0)
        {
            perror("read");
            return;
        }
        total += bytes;
    }
}

int main(){

    //remove(FIFO_NAME);
    while(true) {
        if (mkfifo(FIFO_NAME, (S_IRWXO | S_IRWXG | S_IRWXU))) {
            if (errno == EEXIST) {
                remove(FIFO_NAME);
                std::cout << "Old pipe removed" << std::endl;
                continue;
            }
            perror("mkfifo");
            return 1;
        }
        break;
    }
    int fdPipe;
    if ((fdPipe = open(FIFO_NAME, O_RDWR)) <= 0)
    {
        perror("open");
        return 1;
    }
    std::cout << "Opened new pipe" << std::endl;

    VideoCapture cap(0);
    ConcurrentDeque<Mat> cDeque;
    std::atomic_ulong counter(0);
    FrameSender oddWriter(cDeque, counter, fdPipe, 0, THREADS_NUM);
    FrameSender evenWriter(cDeque, counter, fdPipe, 1, THREADS_NUM);

    if(!cap.isOpened()){
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    char forSync[2];
    //should block here
    std::cout << "Waiting for client..." << std::endl;
    readAll(fdPipe, forSync, 1);
    std::cout << "Client online" << std::endl;

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

/*
        //TEST section
//        size_t fSize = sizeof(*(frame.data));
        int size = frame.total() * frame.elemSize();


//        uchar * tBuf = new uchar[fSize];
        //memcpy(tBuf, frame.data, fSize);
//        Mat frameCopy(frame.rows, frame.cols, frame.type(), (uchar *)tBuf, frame.step);
//        Mat frameCopy(frame.rows, frame.cols, frame.type(), tBuf, frame.step);
        //Mat frameCopy(frame.rows, frame.cols, frame.type(), bytes, frame.step);
        Mat frameCopy(frame.rows, frame.cols, frame.type());
        imshow( "FrameCopy", frameCopy );
        imshow( "Frame", frame );
        //END

        // Press  ESC on keyboard to exit
        auto c = (char)waitKey(25);
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