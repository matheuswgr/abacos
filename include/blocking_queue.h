#ifndef __blocking_queue_h
#define __blocking_queue_h

#include <queue>
#include <mutex>
#include <condition_variable>

namespace abacos
{

    template<typename T>
    class Blocking_Queue
    {
        private:
            std::mutex _sync;

            std::queue<T> _queue;

            std::condition_variable _queue_not_empty;
            std::condition_variable _queue_not_full;

            unsigned int _queue_length;
            const unsigned int _queue_capacity;

        public:
            explicit Blocking_Queue(int capacity) : _queue_capacity(capacity)
            {
                _queue_length = 0;
            }

            void push(T item)
            {
                std::unique_lock<std::mutex> lock(_sync);

                _queue_not_full.wait(lock, [this]
                { return _queue_length < _queue_capacity; });

                _queue.push(item);

                _queue_length++;

                _queue_not_empty.notify_one();
            }

            T pop()
            {
                std::unique_lock<std::mutex> lock(_sync);

                _queue_not_empty.wait(lock, [this]
                { return !_queue.empty(); });

                T item = _queue.front();

                _queue.pop();

                _queue_length--;

                _queue_not_full.notify_one();

                return item;
            }
    };
}

#endif
