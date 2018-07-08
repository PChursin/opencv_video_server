#include <opencv2/core/mat.hpp>
#include "ConcurrentDeque.h"

template<typename T>
T &ConcurrentDeque<T>::pop_front() {
    guard.lock();
    T * value = new T(deque.front());
    deque.pop_front();
    guard.unlock();
    return *value;
}

template<typename T>
T &ConcurrentDeque<T>::pop_back() {
    guard.lock();
    T & value = deque.back();
    deque.pop_back();
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

template class ConcurrentDeque<cv::Mat>;