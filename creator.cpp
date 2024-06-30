#include <iostream>

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <x86intrin.h>

#include "shared.hpp"

static int fd;

static unsigned long long begin_times[EXPECTED_MESSAGE_COUNT];
static unsigned long long end_times[EXPECTED_MESSAGE_COUNT];

void *writer_thread_func(void *param)
{
    shared_uint64_queue *shm = (shared_uint64_queue*)param;

    std::cout << "New writer spawned\n";

    uint32_t cnt = 0;

    unsigned long long begin;

    for (uint64_t i = 0; i < EXPECTED_MESSAGE_COUNT; i++)
    {

        begin = __rdtsc();

        if(shm->q.push_nospin(i))
        {
            // Go away and do something else?
            i--;
            continue;
        }
        else
        {
            begin_times[i] = begin;
            end_times[i] = __rdtsc();
            cnt++;
        }
    }

    std::cout << "Writer completing\n";

    return (void*)cnt;
}

static void begin_write_messages(shared_uint64_queue *shm, uint8_t num_writers)
{
    if (num_writers > MAX_WRITERS)
    {
        std::cout << "Too many writers requested " << num_writers << ", max is: " << MAX_WRITERS << ".\n";
        exit(-1);
    }

    std::cout << "Creating " << num_writers << " writers.\n";

    pthread_t threads[MAX_WRITERS];

    for (int i = 0; i < num_writers; i++)
    {
        if (pthread_create(&threads[i], nullptr, writer_thread_func, shm))
        {
            std::cout << "ERROR: Failed to create thread " << i << " with errno " << errno << ".\n";
            exit(-1);
        }
    }

    uint32_t total_messages = 0;

    for (int i = 0; i < num_writers; i++)
    {
        uint32_t msgcnt = 0;
        pthread_join(threads[i], (void**)&msgcnt);
        std::cout << "Writer " << i << " joined after sending " << msgcnt << " messages.\n";
        total_messages += msgcnt;
    }

    std::cout << "Sent a total of " << total_messages << " messages.\n";

}

static shared_uint64_queue *setup_shared_memory()
{
    int fd = shm_open(SHARED_MEMORY_NAME, 
                        O_CREAT | O_EXCL | O_RDWR, 
                        SHARED_MEMORY_PERM);

    if (fd == -1)
    {
        std::cout << "ERROR: Failed to create shared memory region - errno: " << errno << ".\n";
        shm_unlink(SHARED_MEMORY_NAME);
        return nullptr;
    }

    if (ftruncate(fd, sizeof(shared_uint64_queue)) < -1)
    {
        std::cout << "ERROR: Failed to set size of shared memory region - errno: " << errno << ".\n";
        return nullptr;
    }

    shared_uint64_queue *shm = (shared_uint64_queue *)mmap(NULL, 
                                                            sizeof(shared_uint64_queue), 
                                                            PROT_READ | PROT_WRITE, 
                                                            MAP_SHARED, 
                                                            fd, 
                                                            0);

    if (shm == (void*)-1)
    {
        std::cout << "ERROR: Failed to map shared memory region - errno: " << errno << ".\n";
        return nullptr;
    }

    return shm;
}

static pid_t fork_child_program()
{
    pid_t pid = fork();

    if (pid == -1) // error
    {
        std::cout << "ERROR: Failed to fork a child process with errno: " << errno << ".\n";
        return -1;
    }
    else if (!pid) // child
    {
        static char *newargv[] = { NULL };
        static char *newenv[] = { NULL };
        
        if (execve("./consumer", newargv, newenv ) < 0)
        {
            std::cout << "ERROR: Failed to execv to consumer program with errno: " << errno << ".\n";
            return -1;
        }
    }
    else // parent
    {
        ;
    }

    return pid;
}

int main(int argc, char *argv[])
{
    std::cout << "Starting up shared memory creator\n";

    shared_uint64_queue *shm = setup_shared_memory();

    if (!shm)
        return -1;

    shm->state.w = W_READY;
    shm->state.r = NOT_ALIVE;

    pid_t child;

    if ((child = fork_child_program()) == -1)
        return -1;

    shm->state.wait_for_reader_state(R_READY);
    shm->state.w = WRITING;

    shm->q.head = 0;
    shm->q.tail = 0;

    uint8_t writers;

    if (argc == 1)
        writers = 1;
    else
        writers = strtol(argv[1], nullptr, 10);

    begin_write_messages(shm, writers);

    shm->state.w = W_DONE;

    int child_status = 0;

    while(waitpid(child, &child_status, WNOHANG) != child)
    {
        ;
    }

    std::cout << "Child process " << child << " reaped.\n";

    std::cout << "Creator dumping timing values to disk.\n";
    dump_values("./creator_start_times.txt", begin_times, EXPECTED_MESSAGE_COUNT);
    dump_values("./creator_end_times.txt", end_times, EXPECTED_MESSAGE_COUNT);

    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
