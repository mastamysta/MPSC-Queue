#include <iostream>
#include <array>

#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <x86intrin.h>
#include <unistd.h>
#include <sys/resource.h>

#include "shared.hpp"

void segv_handler(int signum)
{
    std::cout << "ERROR: Consumer encountered a SIGSEGV.\n";
}

static void begin_read_messages(shared_uint64_queue *shm)
{
    unsigned long long int cnt = 0;
    unsigned long long begin;

    while (true)
    {
        uint64_t p;
        

        if(shm->q.pop(p))
        {
            if (shm->state.w == W_DONE)
            {
                std::cout << "head: " << shm->q.head << " tail: " << shm->q.tail << ".\n";
                break;
            }
        }
        else
        {
            cnt++;
        }
    }

    std::cout << "Reader consumed " << cnt << " messages.\n";
}

static shared_uint64_queue *open_shared_queue()
{
    int fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, SHARED_MEMORY_PERM);

    if (fd < 0)
    {
        std::cout << "ERROR: Failed to open shared memory region with errno: " << errno << ".\n";
        return nullptr;
    }

    shared_uint64_queue *shm = (shared_uint64_queue*)mmap(NULL, 
                                                            sizeof(shared_uint64_queue), 
                                                            PROT_READ | PROT_WRITE, 
                                                            MAP_SHARED, 
                                                            fd, 
                                                            0);

    if (shm == (void*)-1)
    {
        std::cout << "ERROR: Failed to map shared memory into consumer with errno: " << errno << ".\n";
        return nullptr;
    }

    return shm;
}

#include <errno.h>

int main()
{
    std::cout << "Consumer started up\n";

    signal(SIGSEGV, segv_handler);

    if (setpriority(PRIO_PROCESS, 0, PRIO_MAX))
    {
        std::cout << "ERROR: Failed to set priority " << errno << ".\n";
    }

    shared_uint64_queue *shm = open_shared_queue();

    if (!shm)
    {
        std::cout << "ERROR: Consumer failed to open shared memory.\n";
        return -1;
    }

    shm->state.r = R_READY;
    shm->state.wait_for_writer_state(WRITING);
    shm->state.r = READING;

    begin_read_messages(shm);

    std::cout << "Consumer done!\n";

    shm->state.r = R_DONE;

    std::cout << "Consumer dumping timing values to disk.\n";

    return 0;
}
