#pragma once

#include <atomic>

// A non-blocking queue which supports multiple writers and one reader.
template <typename T, size_t S>
class fastQueue
{
public:
    fastQueue()
    {
        head = 0;
        tail = 0;
    }

    bool push(T value)
    {
        // Can't push if queue is full
        if (tail == head - 1 || (head == 0 && tail == S-1))
            return true;

        size_t reserved_index = tail;
        size_t next_index = (reserved_index+1) % S;

        while (!tail.compare_exchange_strong(reserved_index, 
                                            next_index,
                                            std::memory_order_release, 
                                            std::memory_order_relaxed))
        {
            reserved_index = tail;

            // Avoid busy-blocking in the event that someone beat us to the last slot.
            if (tail == head - 1 || (head == 0 && tail == S-1))
                return true;
        }

        std::cout << "Writing value " << value << "\n";

        buf[reserved_index] = value;
        valid[reserved_index] = true;

        return false;
    }


    // The fast queue supports multiple writers but only one reader, hence no sync is needed
    // to protect the 'head' offset.
    bool pop(T& ret)
    {
        if (head == tail)
            return true;

        if (!valid[head])
            return true;

        ret = buf[head];
        valid[head] = false;

        head++;

        if (head == S)
            head = 0;

        return false;
    }

    volatile std::atomic<size_t> head;
    volatile std::atomic<size_t> tail;
    volatile std::atomic<T> buf[S];
    volatile bool valid[S];
private:
};
