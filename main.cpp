#include "opencv2/opencv.hpp"
#include <atomic>
#include "FrameSender.h"
#include <sys/stat.h>
#include <zconf.h>

#define THREADS_NUM 2
#define FIFO_NAME "/tmp/mkfifo_opencv"
#define MAX_FRAME_QUEUE 30
#define SLEEP_TIME_USEC 500000

using namespace cv;

void usage(char *arg)
{
    std::cout << "Usage: " << arg << " <filename> [frames]" << std::endl;
    std::cout << " <filename> - path to video file. Use 0 to read stream from web camera" << std::endl;
    std::cout << " [frames] - optional parameter to limit total frames" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 0;
    }

    if (mkfifo(FIFO_NAME, (S_IRWXO | S_IRWXG | S_IRWXU)) && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    std::ofstream pipe(FIFO_NAME, std::ios::out | std::ios::binary);
    if (!pipe.is_open()) {
        std::cout << "Could not open pipe at " << FIFO_NAME << std::endl;
        return 1;
    }
    std::cout << "Opened new pipe" << std::endl;

    VideoCapture cap;
    if (std::string("0") == argv[1])
        cap.open(0);
    else
        cap.open(argv[1]);

    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return 1;
    }

    ConcurrentDeque<Mat> cDeque;
    std::atomic_ulong counter(0);
    FrameSender oddWriter(cDeque, counter, &pipe, 0, THREADS_NUM);
    FrameSender evenWriter(cDeque, counter, &pipe, 1, THREADS_NUM);

    oddWriter.start();
    evenWriter.start();
    ulong frameLimit = argc == 3 ? atol(argv[2]) : (ulong) -1;
    ulong localCounter = 0;
    while (true) {
        //try to limit frames in deque
        if (cDeque.size() > MAX_FRAME_QUEUE) {
            //std::cout << "main sleeps" << std::endl;
            usleep(SLEEP_TIME_USEC);
            continue;
        }
        Mat frame;
        cap >> frame;
        if (frame.empty() || localCounter++ > frameLimit) {
            for (int i = 0; i < THREADS_NUM; i++)
                cDeque.push_back(Mat());
            break;
        }
        cDeque.push_back(frame.clone());
        frame.release();
    }
    oddWriter.wait();
    evenWriter.wait();
    cap.release();
    destroyAllWindows();
    pipe.close();
    return 0;
}