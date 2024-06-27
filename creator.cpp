#include <iostream>

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "shared.h"

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

    strcpy(shm->thestring, "Ringadingding");

    sleep(5);

    std::cout << "Creator woke up and found the shared string has value: " << shm->thestring << ".\n";

    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
