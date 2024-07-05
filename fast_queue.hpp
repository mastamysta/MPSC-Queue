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
        size_t reserved_index = -1;
        size_t next = -1;

        while (true) 
        {
            reserved_index = tail;
            next = (reserved_index + 1) % S;

            if (reserved_index == head - 1 || (head == 0 && reserved_index == S-1))
                return true;

            if (valid[reserved_index])
                return true;

            if(tail.compare_exchange_weak(reserved_index, 
                                            next,
                                            std::memory_order_acquire, 
                                            std::memory_order_acquire))
                break;
        }

        if (valid[reserved_index])
        {
            std::cout << "ERROR: Writing over valid data at index " << reserved_index << ".\n";
        }

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

        head = (head+1 == S) ? 0 : head+1;

        return false;
    }

    volatile std::atomic<size_t> head;
    volatile std::atomic<size_t> tail;
    volatile std::atomic<T> buf[S];
    volatile bool valid[S];
private:
};

// A non-blocking queue which supports multiple writers and one reader.
template <typename T, size_t S>
class fastQueue_nospin
{
public:
    fastQueue_nospin()
    {
        head = 0;
        tail = 0;
        tot = 0;
    }

    bool push(T value)
    {
        size_t ctot = tot.fetch_add(1, std::memory_order_acquire);

        if (ctot >= S)
        {
            tot.fetch_sub(1, std::memory_order_release);
            return true;
        }

        size_t reserved_index = tail.fetch_add(1, std::memory_order_acquire) % S;

        if (reserved_index < 0 || reserved_index > 511)
            std::cout << "ERROR: Limits not working " << reserved_index << ".\n";

        // if (reserved_index == S-1)
        //     tail = 0;

        if (valid[reserved_index])
            std::cout << "ERROR: Overwriting valid data at index " << reserved_index << " with ctot = " << ctot << ".\n";

        buf[reserved_index] = value;
        valid[reserved_index] = true;

        return false;
    }

    bool pop(T& ret)
    {
        if (tot == 0)
            return true;

        if (!valid[head])
            return true; // The current head of the queue is currently being written.

        ret = buf[head];
        valid[head] = false;
        head++;

        if (head == S)
            head = 0;

        tot--;

        return false;
    }

    volatile std::atomic<size_t> head;
    volatile std::atomic<size_t> tail;
    volatile std::atomic<size_t> tot;
    volatile std::atomic<T> buf[S];
    volatile bool valid[S];
private:
};
