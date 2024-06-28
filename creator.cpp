#include <iostream>

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shared.hpp"

static int fd;

static void begin_write_messages(shared_uint64_queue *shm)
{
    for (uint64_t i = 0; i < EXPECTED_MESSAGE_COUNT; i++)
    {
        if(shm->q.push(i))
        {
            // Go away and do something else?
            i--;
            continue;
        }
    }
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

int main()
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

    begin_write_messages(shm);

    shm->state.w = W_DONE;

    int child_status = 0;

    while(waitpid(child, &child_status, WNOHANG) != child)
    {
        ;
    }

    std::cout << "Child process " << child << " reaped.\n";

    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
