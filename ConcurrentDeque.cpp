#include <opencv2/core/mat.hpp>
#include "ConcurrentDeque.h"

template<typename T>
T *ConcurrentDeque<T>::pop_front()
{
    guard.lock();
    T * value = new T(deque.front());
    deque.pop_front();
    guard.unlock();
    return value;
}


template<typename T>
void ConcurrentDeque<T>::push_back(const T &value) {
    guard.lock();
    deque.push_back(value);
    guard.unlock();
}

template<typename T>
void ConcurrentDeque<T>::push_back(T &&value) {
    guard.lock();
    deque.push_back(value);
    guard.unlock();
}

template<typename T>
bool ConcurrentDeque<T>::empty() {
    guard.lock();
    bool res = deque.empty();
    guard.unlock();
    return res;
}

template<class T>
unsigned long ConcurrentDeque<T>::size()
{
    guard.lock();
    ulong res = deque.size();
    guard.unlock();
    return res;
}

template class ConcurrentDeque<cv::Mat>;