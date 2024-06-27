#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "shared.hpp"

int main()
{
    std::cout << "Consumer started up\n";

    int fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, SHARED_MEMORY_PERM);

    if (fd < 0)
    {
        std::cout << "ERROR: Failed to open shared memory region with errno: " << errno << ".\n";
        return -1;
    }

    shared_string *shm = (shared_string*)mmap(NULL, 
                                                sizeof(shared_string), 
                                                PROT_READ | PROT_WRITE, 
                                                MAP_SHARED, 
                                                fd, 
                                                0);

    if (shm == (void*)-1)
    {
        std::cout << "ERROR: Failed to map shared memory into consumer with errno: " << errno << ".\n";
        return -1;
    }

    shm->sync.set_consumer_state(READY);

    std::cout << "Read back the following data from shared memory\n";
    std::cout << shm->thestring << "\n";

    const char *newstring = "Fiddle-dee-dee";
    std::cout << "Setting the shared memory string to " << newstring << ".\n";
    strcpy(shm->thestring, newstring);

    shm->sync.set_consumer_state(DONE);

    return 0;
}