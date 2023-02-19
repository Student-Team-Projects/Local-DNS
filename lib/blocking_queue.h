#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class BlockingQueue {
    private:
        std::mutex mutex;
        std::condition_variable condition;
        std::queue<T> queue;

    public:
        void push(const T&);
        T pop();
};

#include "blocking_queue.tpp"
