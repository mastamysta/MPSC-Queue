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

int main()
{
    std::cout << "Starting up shared memory creator\n";

    int fd = shm_open(SHARED_MEMORY_NAME, 
                        O_CREAT | O_EXCL | O_RDWR, 
                        SHARED_MEMORY_PERM);

    if (fd == -1)
    {
        std::cout << "ERROR: Failed to create shared memory region - errno: " << errno << ".\n";
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    if (ftruncate(fd, sizeof(shared_string)) < -1)
    {
        std::cout << "ERROR: Failed to set size of shared memory region - errno: " << errno << ".\n";
        return -1;
    }

    shared_string *shm = (shared_string *)mmap(NULL, 
                                                sizeof(shared_string), 
                                                PROT_READ | PROT_WRITE, 
                                                MAP_SHARED, 
                                                fd, 
                                                0);

    if (shm == (void*)-1)
    {
        std::cout << "ERROR: Failed to map shared memory region - errno: " << errno << ".\n";
        return -1;
    }

    shm->sync.set_creator_state(READY);
    shm->sync.set_consumer_state(NOT_CREATED);

    strcpy(shm->thestring, "Ringadingding");

    pid_t pid = fork();

    if (pid == -1) // error
    {
        std::cout << "ERROR: Failed to fork a child process with errno: " << errno << ".\n";
        return -1;
    }
    else if (!pid) // child
    {
        if (execve("./consumer", { NULL }, { NULL } ) < 0)
        {
            std::cout << "ERROR: Failed to execv to consumer program with errno: " << errno << ".\n";
            return -1;
        }
    }
    else // parent
    {
        ;
    }

    shm->sync.wait_for_consumer_state(DONE, DONE);

    std::cout << "Creator woke up and found the shared string has value: " << shm->thestring << ".\n";

    int child_status = 0;

    while(waitpid(pid, &child_status, WNOHANG) != pid)
    {
        ;
    }

    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
