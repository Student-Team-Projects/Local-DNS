
template <typename T>
void BlockingQueue<T>::push(const T& value) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(value);
    }
    condition.notify_one();
}

template <typename T>
T BlockingQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [=, this]{ return !queue.empty(); });
    T value(std::move(queue.front()));
    queue.pop();
    return value;
}
