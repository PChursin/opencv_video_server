#include "FrameSender.h"

void sendMat(cv::Mat *mat, std::ofstream *pipe);

void threadCycle(ConcurrentDeque<cv::Mat> &cDeque, std::atomic_ulong &counter, std::ofstream *pipe,
                 int order, int totalThreads)
{
    std::chrono::duration<int, std::milli> sleepTime(50);
    while (true)
    {
        int tmp = counter.load() % totalThreads;
        if (tmp == order) {
            if (cDeque.empty())
                continue;
            cv::Mat *frame = cDeque.pop_front();
            //uncomment to check sequence at runtime
            //std::cout << order << " " << counter.load() << std::endl;
            sendMat(frame, pipe);
            counter++;
            if (frame->empty()) {
                delete frame;
                break;
            }
            delete frame;
        } else
            std::this_thread::sleep_for(sleepTime);
    }
}

void sendMat(cv::Mat *mat, std::ofstream *pipe)
{
    int matType = mat->type();
    char *sBuf = (char *) mat->data;
    int matSize = mat->total() * mat->elemSize();
    pipe->write((char *) &mat->rows, sizeof(int));
    pipe->write((char *) &mat->cols, sizeof(int));
    pipe->write((char *) &matType, sizeof(int));
    pipe->write((char *) &matSize, sizeof(int));
    pipe->write(sBuf, matSize);
    pipe->flush();
}

FrameSender::FrameSender(ConcurrentDeque<cv::Mat> &cDeque, std::atomic_ulong &counter, std::ofstream *pipe, int order,
                         int totalThreads)
        : concurrentDeque(cDeque), counter(counter), pipe(pipe), order(order), totalThreads(totalThreads)
{

}

void FrameSender::start() {
    thread = new std::thread(threadCycle, std::ref(concurrentDeque), std::ref(counter), pipe, order, totalThreads);
}

void FrameSender::wait() {
    thread->join();
}

FrameSender::~FrameSender() {
    delete thread;
}
