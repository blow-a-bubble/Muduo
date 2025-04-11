#include <iostream>
#include <sys/timerfd.h>
#include <cstdint>
#include <unistd.h>

int main()
{
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(fd < 0)
    {
        perror("timerfd_create err");
        return 1;
    }
    struct itimerspec its;
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;//设置第一次超时时间为1s

    its.it_interval.tv_sec = 3;
    its.it_interval.tv_nsec = 0;//设置第一次之后超时时间为1s

    timerfd_settime(fd, 0, &its, nullptr);
    uint64_t count = 0;
    while(true)
    {
        ssize_t n = read(fd, &count, sizeof(count));
        std::cout << n << std::endl;
        std::cout << count << std::endl;
    }
    return 0;
}
