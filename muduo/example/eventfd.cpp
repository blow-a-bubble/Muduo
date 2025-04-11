#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>


int main()
{
    int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    uint64_t val = 2;
    int n = write(fd, &val, sizeof(val));
    n = write(fd, &val, sizeof(val));
    n = write(fd, &val, sizeof(val));

    if(n <= 0)
    {
        return -1;
    }
    uint64_t ret = 0;
    n = read(fd, &ret, sizeof(ret));
    std::cout << ret << std::endl;
    return 0;
}