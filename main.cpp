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
#include <fstream>

#define THREADS_NUM 2
#define FIFO_NAME "/tmp/mkfifo_opencv"

static const int ESC_CODE = 27;
using namespace cv;

void sendAll(int fdPipe, char * buf, int len);

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

void usage(char* arg)
{
    std::cout << "Usage: " << arg << " <filename> [frames]" << std::endl;
    std::cout << " <filename> - path to video file. Use 0 to read stream from web camera" << std::endl;
    std::cout << " [frames] - optional parameter to limit total frames" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3)
    {
        usage(argv[0]);
        return 0;
    }

    while(true) 
    {
        //if (mkfifo(FIFO_NAME, (S_IRWXO | S_IRWXG | S_IRWXU)))
        if (mkfifo(FIFO_NAME, 0666))
        {
            if (errno == EEXIST)
            {
                remove(FIFO_NAME);
                std::cout << "Old pipe removed" << std::endl;
                continue;
            }
            perror("mkfifo");
            return 1;
        }
        break;
    }/*
    std::fstream pipe(FIFO_NAME, std::ios::in | std::ios::out | std::ios::binary);
    if (!pipe.is_open())
    {
        std::cout << "Could not open pipe at " << FIFO_NAME << std::endl;
        return 1;
    }*/
    int fdPipe;
    if ((fdPipe = open(FIFO_NAME, O_RDWR)) <= 0)
    {
        perror("open");
        return 1;
    }
    std::cout << "Opened new pipe" << std::endl;

    VideoCapture cap;
    if (std::string("0") == argv[1])
        cap.open(0);
    else
        cap.open(argv[1]);

    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream or file" << std::endl;
        return 1;
    }

    ConcurrentDeque<Mat> cDeque;
    std::atomic_ulong counter(0);
    FrameSender oddWriter(cDeque, counter, fdPipe, 0, THREADS_NUM);
    FrameSender evenWriter(cDeque, counter, fdPipe, 1, THREADS_NUM);

    char * forSync = new char[8];
    std::cout << "Waiting for client..." << std::endl;
    //will block here
    readAll(fdPipe, forSync, 1);
    strcpy(forSync, "S");
    sendAll(fdPipe, forSync, 1);
    std::cout << "Client online" << std::endl;
    delete [] forSync;

    oddWriter.start();
    evenWriter.start();
    ulong frameLimit = argc == 3 ? atol(argv[2]) : (ulong)-1;
    ulong localCounter = 0;
    while(true){
        Mat frame;
        cap >> frame;
        if (frame.empty() || localCounter++ > frameLimit)
        {
            for (int i = 0; i < THREADS_NUM; i++)
                cDeque.push_back(Mat());
            break;
        }
        cDeque.push_back(frame);
    }
    oddWriter.wait();
    evenWriter.wait();
    cap.release();
    destroyAllWindows();
    close(fdPipe);
    return 0;
}