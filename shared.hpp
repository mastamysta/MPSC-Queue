#pragma once

#include <atomic>

#define SHARED_MEMORY_NAME "/bruh"
#define SHARED_MEMORY_SIZE 1024
#define SHARED_MEMORY_PERM 0600

enum state
{
    NOT_CREATED,
    READY,
    WAITING_FOR_CONSUMER,
    WAITING_FOR_CREATOR,
    DONE
};


class event_blob
{
public:
    void set_creator_state(int new_creator_state)
    {
        creator_state = new_creator_state;
    }

    void set_consumer_state(int new_consumer_state)
    {
        consumer_state = new_consumer_state;
    }

    void wait_for_creator_state(int wait_creator_state, int new_consumer_state)
    {
        while (creator_state != wait_creator_state)
        {
            sched_yield();
        }

        consumer_state = new_consumer_state;
    }

    void wait_for_consumer_state(int wait_consumer_state, int new_creator_state)
    {
        while (consumer_state != wait_consumer_state)
        {
            sched_yield();
        }

        creator_state = new_creator_state;
    }

private:
    std::atomic<int> creator_state;
    std::atomic<int> consumer_state;

};

struct shared_string
{
    event_blob sync;
    char thestring[128];
};
