#ifndef __blocking_queue_h
#define __blocking_queue_h

#include <queue>
#include <mutex>
#include <condition_variable>

namespace abacos
{

    template <typename T>
    class Blocking_Queue
    {
    public:
        explicit Blocking_Queue(size_t capacity)
            : capacity_(capacity) {}

        void push(T item)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            not_full_.wait(lock, [&]
                           { return queue_.size() < capacity_; });

            queue_.push(std::move(item));
            lock.unlock();
            not_empty_.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> lock(mtx_);
            not_empty_.wait(lock, [&]
                            { return !queue_.empty(); });

            T item = queue_.front();
            queue_.pop();
            lock.unlock();
            not_full_.notify_one();
            return item;
        }

    private:
        std::mutex mtx_;
        std::condition_variable not_empty_;
        std::condition_variable not_full_;
        std::queue<T> queue_;
        size_t capacity_;
    };

}

#endif
