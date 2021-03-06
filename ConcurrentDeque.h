#ifndef OPENCV_VIDEO_SERVER_CONCURRENTDEQUE_H
#define OPENCV_VIDEO_SERVER_CONCURRENTDEQUE_H

#include <mutex>
#include <deque>

template<class T>
class ConcurrentDeque
{
public:
    ConcurrentDeque() { };

    T *pop_front();
    void push_back(const T& value);
    void push_back(T&& value);
    bool empty();

    unsigned long size();
private:
    std::mutex guard;
    std::deque<T> deque;
};

#endif //OPENCV_VIDEO_SERVER_CONCURRENTDEQUE_H
