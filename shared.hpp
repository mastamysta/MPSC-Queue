#pragma once

#include <atomic>

#include "fast_queue.hpp"

#define SHARED_MEMORY_NAME "/bruh"
#define SHARED_MEMORY_SIZE 1024
#define SHARED_MEMORY_PERM 0600

#define EXPECTED_MESSAGE_COUNT 100000

enum writer_state
{
    W_READY,
    WRITING,
    W_DONE
};

enum reader_state
{
    NOT_ALIVE,
    R_READY,
    READING,
    R_DONE
};

class agent_states
{    
public:
    volatile writer_state w;
    volatile reader_state r;

    void wait_for_writer_state(writer_state s)
    {
        while (w != s) { ; }
    }

    void wait_for_reader_state(reader_state s)
    {
        while (r != s) { ; }
    }
};

struct shared_uint64_queue
{
    agent_states state;
    fastQueue<uint64_t, 512> q;
};
