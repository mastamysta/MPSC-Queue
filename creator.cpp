#include <iostream>

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        return -1;
    }
}
