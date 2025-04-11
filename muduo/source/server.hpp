#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <memory>
#include <typeinfo>
#include <condition_variable>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <signal.h>

#define BUFFER_DEFAULT_SIZE 1024
#define INFO 0
#define DEBUG 1
#define ERR 2
#define LOG_LEVEL DEBUG
#define LOG(level, format, ...)                                                                                                      \
    do                                                                                                                               \
    {                                                                                                                                \
        if (level < LOG_LEVEL)                                                                                                       \
            break;                                                                                                                   \
        time_t current_time = time(nullptr);                                                                                         \
        struct tm *ltm = localtime(&current_time);                                                                                   \
        char timestr[32] = {0};                                                                                                      \
        strftime(timestr, 31, "%H:%M:%S", ltm);                                                                                      \
        if (level == INFO)                                                                                                           \
            fprintf(stdout, "[INFO][%p %s %s:%d]" format "\n", (void *)pthread_self(), timestr, __FILE__, __LINE__, ##__VA_ARGS__);  \
        if (level == DEBUG)                                                                                                          \
            fprintf(stdout, "[DEBUG][%p %s %s:%d]" format "\n", (void *)pthread_self(), timestr, __FILE__, __LINE__, ##__VA_ARGS__); \
        if (level == ERR)                                                                                                            \
            fprintf(stdout, "[ERR][%p %s %s:%d]" format "\n", (void *)pthread_self(), timestr, __FILE__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define INFO_LOG(format, ...) LOG(INFO, format, ##__VA_ARGS__)
#define DEBUG_LOG(format, ...) LOG(DEBUG, format, ##__VA_ARGS__)
#define ERR_LOG(format, ...) LOG(ERR, format, ##__VA_ARGS__)
class Buffer
{
private:
    std::vector<char> _buf;
    uint64_t _read_idx;
    uint64_t _write_idx;

public:
    Buffer(uint64_t size = BUFFER_DEFAULT_SIZE) : _buf(size), _read_idx(0), _write_idx(0)
    {
    }
    // 获取初始位置
    char *Begin()
    {
        return &*_buf.begin();
    }
    // 获取读位置
    char *ReadPosition()
    {
        return &_buf[_read_idx];
    };
    // 获取写位置
    char *WritePosition()
    {
        return &_buf[_write_idx];
    };
    // 获取缓冲区头部空闲大小
    uint64_t HeadIdleSize()
    {
        return _read_idx;
    }
    // 获取缓冲区末尾空闲大小
    uint64_t TailIdleSize()
    {
        return _buf.size() - _write_idx;
    }
    // 获取可读数据大小
    uint64_t ReadAbleSize()
    {
        return _write_idx - _read_idx;
    }
    // 将读位置向后移动
    void MoveReadBack(uint64_t size)
    {
        assert(size <= ReadAbleSize());
        _read_idx += size;
    }
    // 将写位置向后移动
    void MoveWriteBack(uint64_t size)
    {
        assert(size <= TailIdleSize());
        _write_idx += size;
    }
    // 确保可以写入
    void EnsureWriteAble(uint64_t size)
    {
        // 缓冲区尾部大小就足够
        if (size <= TailIdleSize())
            return;

        // 缓冲区剩余大小足够
        if (size <= HeadIdleSize() + TailIdleSize())
        {
            // 把当前可读数据大小保存起来
            size_t sz = ReadAbleSize();
            std::copy(ReadPosition(), ReadPosition() + ReadAbleSize(), Begin());
            _read_idx = 0;
            _write_idx = sz;
        }
        // 缓冲区大小不够，需要增容
        else
        {
            _buf.resize(_write_idx + size);
        }
    }
    // 读取
    void Read(char *buf, uint64_t size)
    {
        // 确保读取大小合法
        assert(size <= ReadAbleSize());
        std::copy(ReadPosition(), ReadPosition() + size, buf);
    }
    // 读取而且弹出数据
    void ReadAndPop(char *buf, uint64_t size)
    {
        // 确保读取大小合法
        assert(size <= ReadAbleSize());
        Read(buf, size);
        MoveReadBack(size);
    }

    // 以string类型读取数据
    std::string ReadAsString(uint64_t size)
    {
        assert(size <= ReadAbleSize());
        std::string str;
        str.resize(size);
        Read(&str[0], size);
        return str;
    }
    // 以string类型读取数据而且弹出
    std::string ReadAsStringAndPop(uint64_t size)
    {
        assert(size <= ReadAbleSize());
        std::string str;
        str = ReadAsString(size);
        MoveReadBack(size);
        return str;
    }
    // 写入
    void Write(const char *buf, uint64_t size)
    {
        EnsureWriteAble(size);
        std::copy(buf, buf + size, WritePosition());
    }
    // 写入而且推入数据
    void WriteAndPush(const char *buf, uint64_t size)
    {
        if (size == 0)
            return;
        EnsureWriteAble(size);
        Write(buf, size);
        MoveWriteBack(size);
    }
    // 写入string类型数据
    void WriteString(const std::string &str)
    {
        Write(str.c_str(), str.size());
    }
    // 写入string类型数据而且推入
    void WriteStringAndPush(const std::string &str)
    {
        WriteString(str);
        MoveWriteBack(str.size());
    }

    // 写入buffer类型数据
    void WriteBuffer(Buffer &buffer)
    {
        uint64_t size = buffer.ReadAbleSize();
        Write(buffer.ReadPosition(), size);
    }

    // 写入buffer类型数据并且推入
    void WriteBufferAndPush(Buffer &buffer)
    {
        WriteBuffer(buffer);
        MoveWriteBack(buffer.ReadAbleSize());
    }
    // 清空缓冲区
    void Clear()
    {
        _read_idx = _write_idx = 0;
    }

    // 获取\n位置
    char *FindCRLF()
    {
        char *ret = (char *)memchr(ReadPosition(), '\n', ReadAbleSize());
        return ret;
    }
    // 获取一行数据
    std::string GetOneLine()
    {
        char *pos = FindCRLF();
        // 没有'\n'，获取失败返回空字符串
        if (pos == nullptr)
            return "";
        // 把\n也读出来
        std::string ret(ReadPosition(), pos + 1);
        return ret;
    }

    // 获取一行数据并且弹出
    std::string GetOneLineAndPop()
    {
        std::string ret = GetOneLine();
        MoveReadBack(ret.size());
        return ret;
    }
};

#define BACKLOG 1024
class Socket
{
private:
    int _sockfd;

public:
    Socket() : _sockfd(-1) {}
    Socket(int fd) : _sockfd(fd) {}
    ~Socket()
    {
        Close();
    }
    int Fd()
    {
        return _sockfd;
    }
    // 创建套接字
    bool CreateSocket()
    {
        // int socket(int domain, int type, int protocol);
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            ERR_LOG("create socket fatal, errstring: %s", strerror(errno));
            return false;
        }
        return true;
    }
    // 绑定
    bool Bind(const std::string &ip, uint16_t port)
    {
        // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        memset(&peer, 0, len);
        peer.sin_family = AF_INET;
        peer.sin_addr.s_addr = inet_addr(ip.c_str());
        peer.sin_port = htons(port);
        int ret = bind(_sockfd, (struct sockaddr *)&peer, len);
        if (ret < 0)
        {
            DEBUG_LOG("port: %d", port);
            ERR_LOG("socket bind fatal, errstring: %s", strerror(errno));
            return false;
        }
        return true;
    }
    // 设置监听状态
    bool Listen(int backlog = BACKLOG)
    {
        // int listen(int sockfd, int backlog);
        int ret = listen(_sockfd, backlog);
        if (ret < 0)
        {
            ERR_LOG("socket listen fatal, errstring: %s", strerror(errno));
            return false;
        }
        return true;
    }
    // 获取套接字  ---这里不考虑获取ip和端口
    int Accept()
    {
        // int accept(int sockfd, struct sockaddr *_Nullable restrict addr,socklen_t *_Nullable restrict addrlen);
        int newsock = accept(_sockfd, nullptr, nullptr);
        if (newsock < 0)
        {
            ERR_LOG("socket accept fatal, errstring: %s", strerror(errno));
            return -1;
        }
        return newsock;
    }
    // 连接服务器
    bool Connect(const std::string ip, uint16_t port)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        memset(&peer, 0, len);
        peer.sin_family = AF_INET;
        peer.sin_addr.s_addr = inet_addr(ip.c_str());
        peer.sin_port = htons(port);
        int n = connect(_sockfd, (struct sockaddr *)&peer, len);
        if (n < 0)
        {
            ERR_LOG("socket connect fatal, errstring: %s", strerror(errno));
            return false;
        }
        return true;
    }
    // 接受数据
    ssize_t Recv(void *buffer, size_t len, int flag = 0)
    {
        // ssize_t recv(int sockfd, void buf[.len], size_t len, int flags);
        ssize_t ret = recv(_sockfd, buffer, len, flag);
        if (ret <= 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            else
            {
                ERR_LOG("socket recv err, errstring: %s", strerror(errno));
                return -1;
            }
        }
        return ret;
    }
    // 接受数据非阻塞
    ssize_t RecvNonBlock(void *buffer, size_t len)
    {
        // ssize_t recv(int sockfd, void buf[.len], size_t len, int flags);
        ssize_t ret = recv(_sockfd, buffer, len, MSG_DONTWAIT);
        if (ret <= 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            else if(ret == 0)
            {
                return -1;
            }
            else
            {
                ERR_LOG("socket recv err");
                return -1;
            }
        }
        return ret;
    }
    // 发送数据
    ssize_t Send(const void *buffer, size_t len, int flag = 0)
    {
        // ssize_t send(int sockfd, const void buf[.len], size_t len, int flags);
        ssize_t ret = send(_sockfd, buffer, len, flag);
        if (ret <= 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            else
            {
                ERR_LOG("socket send err");
                return -1;
            }
        }
        return ret;
    }
    // 发送数据非阻塞
    ssize_t SendNonBlock(const void *buffer, size_t len)
    {
        // ssize_t send(int sockfd, const void buf[.len], size_t len, int flags);
        // DEBUG_LOG("buffer: %s", (char*)buffer);
        ssize_t ret = send(_sockfd, buffer, len, MSG_DONTWAIT);
        if (ret <= 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            else
            {
                ERR_LOG("socket send err, errstring: %s", strerror(errno));
                return -1;
            }
        }
        return ret;
    }
    // 关闭套接字
    void Close()
    {
        if (_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }
    // 创建服务器
    bool CreateServer(uint16_t port, const std::string &ip = "0.0.0.0")
    {
        if (CreateSocket() == false)
            return false;
        // NonBlock();
        ReUseAddress();
        if (Bind(ip, port) == false)
            return false;
        if (Listen() == false)
            return false;
        return true;
    }
    // 创建客户端
    bool CreateClient(uint16_t port, const std::string &ip)
    {
        if (CreateSocket() == false)
            return false;
        if (Connect(ip, port) == false)
            return false;
        return true;
    }
    // 设置非阻塞
    void NonBlock()
    {
        int fl = fcntl(_sockfd, F_GETFL);
        fcntl(_sockfd, F_SETFL, fl | O_NONBLOCK);
    }
    // 设置ip port复用
    void ReUseAddress()
    {
        int opt = 1;
        int ret = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        if (ret < 0)
        {
            ERR_LOG("setsockopt fatal");
        }
    }
};

class Poller;    // 对Poller进行声明
class EventLoop; // 对EventLoop进行声明
// 对文件描述符的事件进行管理
class Channel
{
    using EventCallBack = std::function<void()>;

private:
    int _fd;
    EventLoop *_loop;
    uint32_t _events;  // 管理的事件
    uint32_t _revents; // 就绪的事件

    EventCallBack _read_callback;
    EventCallBack _write_callback;
    EventCallBack _err_callback;
    EventCallBack _close_callback;
    EventCallBack _event_callback;

public:
    Channel(int fd, EventLoop *loop)
        : _fd(fd), _events(0), _revents(0), _loop(loop)
    {
    }
    // 获取文件描述符
    int Fd()
    {
        return _fd;
    }
    // 获取管理的事件
    uint32_t GetEvent()
    {
        return _events;
    }
    // 设置管理的事件
    void SetEvent(uint32_t events)
    {
        _events = events;
    }
    // 获取就绪的事件
    uint32_t GetREvent()
    {
        return _revents;
    }
    // 设置就绪的事件
    void SetREvent(uint32_t revents)
    {
        _revents = revents;
    }
    // 是否监视了读事件
    bool ReadAble()
    {
        return _events & EPOLLIN;
    }
    // 是否监视了写事件
    bool WriteAble()
    {
        return _events & EPOLLOUT;
    }
    // 启动可读监控
    void EnableRead()
    {
        _events |= EPOLLIN;
        Update();
    }
    // 启动可写监控
    void EnableWrite()
    {
        _events |= EPOLLOUT;
        Update();
    }
    // 关闭可读监控
    void DisableRead()
    {
        _events &= (~EPOLLIN);
        Update();
    }
    // 关闭可写监控
    void DisableWrite()
    {
        _events &= (~EPOLLOUT);
        Update();
    }
    // 取消所有事件监控
    void DisableAll()
    {
        _events = 0;
        Update();
    }
    void Update();
    // {
    //     _epoll->UpdateEvent(this);
    // }
    // 移除监控
    void Remove();
    // {
    //     _epoll->RemoveEvent(this);
    // }
    // 设置可读事件回调
    void SetReadCallBack(const EventCallBack &read_callback)
    {
        _read_callback = read_callback;
    }
    // 设置可写事件回调
    void SetWriteCallBack(const EventCallBack &write_callback)
    {
        _write_callback = write_callback;
    }
    // 设置错误事件回调
    void SetErrCallBack(const EventCallBack &err_callback)
    {
        _err_callback = err_callback;
    }
    // 设置关闭事件回调
    void SetCloseCallBack(const EventCallBack &close_callback)
    {
        _close_callback = close_callback;
    }
    // 设置任意事件回调
    void SetEventCallBack(const EventCallBack &event_callback)
    {
        _event_callback = event_callback;
    }
    // 处理事件
    void HanlderEvent()
    {
        if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
        {
            // 先处理读事件，再进行活跃度刷新
            if (_read_callback)
                _read_callback();
        }

        if (_revents & EPOLLOUT)
        {
            // 先处理写事件，再进行活跃度刷新
            if (_write_callback)
                _write_callback();
        }
        else if (_revents & EPOLLERR)
        {
            if (_err_callback)
                _err_callback();
        }
        else if (_revents & EPOLLHUP)
        {
            if (_close_callback)
                _close_callback();
        }
        //
        if (_event_callback)
        {
            _event_callback();
        }
    }
};

#define MAX_EPOLL_EVENT 1024
class Poller
{
private:
    int _epfd;                                   // epoll 句柄
    struct epoll_event _events[MAX_EPOLL_EVENT]; // 事件
    std::unordered_map<int, Channel *> _channels;

private:
    void Update(Channel *channel, int op)
    {
        int fd = channel->Fd();
        struct epoll_event ep;
        memset(&ep, 0, sizeof(ep));
        ep.data.fd = fd;
        ep.events = channel->GetEvent();
        epoll_ctl(_epfd, op, fd, &ep);
    }
    bool IsInChannels(int fd)
    {
        auto it = _channels.find(fd);
        if (it == _channels.end())
        {
            return false;
        }
        return true;
    }

public:
    Poller()
    {
        _epfd = epoll_create(MAX_EPOLL_EVENT);
        if (_epfd < 0)
        {
            ERR_LOG("epoll_create fatal, errstring: %s", strerror(errno));
            abort(); // 信号的方式终止进程
        }
        memset(_events, 0, sizeof(_events));
    }
    // 添加或更改事件监控
    void UpdateEvent(Channel *channel)
    {
        int fd = channel->Fd();
        // 如果不存在就添加监控
        if (IsInChannels(fd) == false)
        {
            Update(channel, EPOLL_CTL_ADD);
            _channels[fd] = channel;
        }
        // 如果存在就添加监控
        else
        {
            Update(channel, EPOLL_CTL_MOD);
        }
    }
    // 删除事件监控
    void RemoveEvent(Channel *channel)
    {
        int fd = channel->Fd();
        assert(_channels.find(fd) != _channels.end());
        Update(channel, EPOLL_CTL_DEL);
        _channels.erase(fd);
    }
    // 进行事件监控
    void Poll(std::vector<Channel *> *channels)
    {
        // 阻塞式等待 timeout = -1
        int nfds = epoll_wait(_epfd, _events, MAX_EPOLL_EVENT, -1);
        if (nfds < 0)
        {
            if (errno == EINTR)
            {
                return;
            }
            ERR_LOG("epoll_wait fatal, errstring: %s", strerror(errno));
        }
        // 将就绪的事件打捞给上层
        for (int i = 0; i < nfds; ++i)
        {
            int fd = _events[i].data.fd;
            uint32_t revent = _events[i].events;
            auto pos = _channels.find(fd);
            assert(pos != _channels.end());
            Channel *channel = pos->second;
            channel->SetREvent(revent);
            channels->push_back(channel);
        }
    }
};

using task_t = std::function<void()>;
using release_t = std::function<void()>;
// 对事件进行封装
// 备选方案：std::enable_shared_from_this<TimerTask>替代weak_ptr
class TimerTask : public std::enable_shared_from_this<TimerTask>
{
private:
    uint64_t _id;          // 事件id
    task_t _task_cb;       // 任务回调
    int _time;             // 定时器时长
    bool _run;             // 是否启动事件定时器
    release_t _release_cb; // 定时器触发后的释放回调
public:
    TimerTask(uint64_t id, task_t task_cb, int timeout)
        : _id(id), _task_cb(task_cb), _run(true), _time(timeout) {}
    ~TimerTask()
    {
        if (_run)
        {
            _task_cb();
        }
        _release_cb();
    }
    void SetRelease(release_t release)
    {
        _release_cb = release;
    }
    void Cancel()
    {
        _run = false;
    }
    int GetDelay()
    {
        return _time;
    }
};

// 时间轮
class TimerWheel
{
private:
    using PtrTask = std::shared_ptr<TimerTask>;
    using WeakTask = std::weak_ptr<TimerTask>;
    int _capacity;                                        // 一轮最大时长
    std::vector<std::vector<PtrTask>> _wheel;             // 时间轮
    int _step;                                            // 步调
    std::unordered_map<uint64_t, WeakTask> _id_timer_map; // id和定时器映射表
    EventLoop *_loop;                                     // 通过EventLoop来管理时间轮
    int _timer_fd;                                        // timerfd对象
    std::unique_ptr<Channel> _timer_fd_channel;           // 管理timerfd的channel
    // 创建TimerFd文件描述符
    static int CreateTimerFd()
    {
        int fd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (fd < 0)
        {
            ERR_LOG("timerfd_create fatal, errstring: %s", strerror(errno));
            abort();
        }
        struct itimerspec its;
        its.it_value.tv_sec = 1;
        its.it_value.tv_nsec = 0; // 设置第一次超时时间为1s

        its.it_interval.tv_sec = 1;
        its.it_interval.tv_nsec = 0; // 设置第一次之后超时时间为1s

        int n = timerfd_settime(fd, 0, &its, nullptr);
        if (n < 0)
        {
            ERR_LOG("timerfd_settime fatal, errstring: %s", strerror(errno));
            abort();
        }
        return fd;
    }

    // 添加定时任务
    void TimerAddInLoop(uint64_t id, task_t task, int timeout)
    {
        // 定时器不能超过时间轮范围
        if (timeout < 0 || timeout > _capacity)
            return;
        PtrTask timer(new TimerTask(id, task, timeout));
        timer->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
        int pos = (_step + timeout) % _capacity;
        // DEBUG_LOG("%d", pos);
        _wheel[pos].push_back(timer);
        _id_timer_map[id] = WeakTask(timer);
        // std::cout << "TimerAddInLoop: " << id << std::endl;
    }
    // 刷新活跃度
    void TimerReFreshInLoop(uint64_t id)
    {
        auto it = _id_timer_map.find(id);
        if (it == _id_timer_map.end())
        {
            // 如果定时任务不存在，就不需要刷新了
            return;
        }
        if (_id_timer_map[id].expired())
            return;
        auto timer = _id_timer_map[id].lock();
        int timeout = timer->GetDelay();
        int pos = (_step + timeout) % _capacity;
        _wheel[pos].push_back(timer);
        // INFO_LOG("TimerReFreshInLoop: %ld", id);
    }

    // 取消定时任务
    void TimerCancelInLoop(uint64_t id)
    {
        auto it = _id_timer_map.find(id);
        if (it == _id_timer_map.end())
        {
            // 如果定时任务不存在，就不需要刷新了
            return;
        }
        if (_id_timer_map[id].expired())
            return;
        auto timer = _id_timer_map[id].lock();
        timer->Cancel();
    }
    uint64_t ReadTimerFd()
    {
        // INFO_LOG("ReadTimerFd ....");
        uint64_t val;
        int n = read(_timer_fd, &val, sizeof(val));
        if (n < 0)
        {
            ERR_LOG("ReadTimerFd fatal, errstring: %s", strerror(errno));
            abort();
        }
        return val;
    }
    void OnTime()
    {
        uint64_t n = ReadTimerFd();
        for (uint64_t i = 0; i < n; ++i)
        {
            RunTimerWheel();
        }
    }

public:
    TimerWheel(EventLoop *loop, int capacity = 60)
        : _capacity(capacity), _wheel(capacity), _step(0),
          _loop(loop), _timer_fd(CreateTimerFd()), _timer_fd_channel(new Channel(_timer_fd, _loop))
    {
        _timer_fd_channel->SetReadCallBack(std::bind(&TimerWheel::OnTime, this));
        _timer_fd_channel->EnableRead();
    }
    void RemoveTimer(uint64_t id)
    {
        auto it = _id_timer_map.find(id);
        if (it == _id_timer_map.end())
        {
            // 如果定时任务不存在，就不需要移除了
            return;
        }
        _id_timer_map.erase(it);
    }

    // 添加任务的定时器,_id_timer_map的访问存在线程安全，如果不想加锁的话就放在一个线程内执行
    void TimerAdd(uint64_t id, task_t task, int time);
    // 刷新活跃度
    void TimerReFresh(uint64_t id);

    // 取消定时任务
    void TimerCancel(uint64_t id);

    // 是否存在该定时器,用于取消定时任务后又再次开启
    // 这里可能有线程安全问题，这个接口不能被外界使用者调用，只能在eventloop对应的线程内执行
    bool HasTimer(uint64_t id)
    {
        auto it = _id_timer_map.find(id);
        if (it == _id_timer_map.end())
            return false;
        return true;
    }
    // 启动时间轮
    void RunTimerWheel()
    {
        // std::cout << "--------" << std::endl;
        // sleep(1);
        _step++;
        _step %= _capacity;
        _wheel[_step].clear();
        // DEBUG_LOG("step: %d", _step);
    }
};

class EventLoop
{
private:
    using Functor = std::function<void()>;
    std::thread::id _id;                        // EventLoop绑定的线程
    int _event_fd;                              // eventfd唤醒IO事件监控有可能导致的阻塞
    std::unique_ptr<Channel> _event_fd_channel; // 管理_event_fd的channel
    Poller _poller;                             // 事件管理对象
    std::vector<Functor> _tasks;                // 任务池
    std::mutex _mutex;                          // 任务池锁
    TimerWheel _timerwheel;                     // 定时任务管理器
    // 执行任务池中所有的任务
    void RunAllTask()
    {
        std::vector<Functor> tasks;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            tasks.swap(_tasks);
        }
        for (auto &e : tasks)
        {
            e();
        }
    }
    // 创建eventfd对象
    int CreateEventFd()
    {
        int ret = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (ret < 0)
        {
            ERR_LOG("eventfd fatal, errstring: %s", strerror(errno));
            abort();
        }
        return ret;
    }
    // 从eventfd中读
    void ReadEventFd()
    {
        uint64_t val = 0;
        int n = read(_event_fd, &val, sizeof(val));
        {
            if (n < 0)
            {
                ERR_LOG("ReadEventFd fatal, errstring: %s", strerror(errno));
                abort();
            }
        }
    }
    // 唤醒poller->poll
    void WakeEventFd()
    {
        uint64_t val = 1;
        int n = write(_event_fd, &val, sizeof(val));
        // DEBUG_LOG("_event_fd: %d", _event_fd);
        // DEBUG_LOG("WakeEventFd");
        {
            if (n < 0)
            {
                ERR_LOG("WakeEventFd fatal, errstring: %s", strerror(errno));
                abort();
            }
        }
    }

public:
    EventLoop()
        : _id(std::this_thread::get_id()),
          _event_fd(CreateEventFd()),
          _event_fd_channel(new Channel(_event_fd, this)),
          _timerwheel(this)
    {
        _event_fd_channel->SetReadCallBack(std::bind(&EventLoop::ReadEventFd, this));
        _event_fd_channel->EnableRead();
    }
    // 运行
    void Start()
    {
        while (1)
        {
            // 1.poller监控 2.处理监控事件 3.执行任务队列中的任务
            std::vector<Channel *> channels;
            _poller.Poll(&channels);
            for (auto &e : channels)
            {
                e->HanlderEvent();
            }
            RunAllTask();
        }
    }
    // 判断当前线程是否是EventLoop绑定的线程
    bool IsInLoop()
    {
        return std::this_thread::get_id() == _id;
    }

    void AssertInLoop()
    {
        assert(std::this_thread::get_id() == _id);
    }
    // 将任务添加到任务池中
    void QueueInLoop(const Functor &func)
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks.push_back(func);
        }
        // 其实就是给eventfd写一个数据，触发eventloop中的事件监听处理，然后后续好处理任务池
        WakeEventFd();
    }
    // 在EventLoop绑定的线程中运行任务
    void RunInLoop(const Functor &func)
    {
        if (IsInLoop())
        {
            return func();
        }
        else
        {
            QueueInLoop(func);
        }
    }

    // 更新对事件的监控
    void UpdateEvent(Channel *channel)
    {
        _poller.UpdateEvent(channel);
    }
    // 移除监控
    void RemoveEvent(Channel *channel)
    {
        _poller.RemoveEvent(channel);
    }

    // 添加定时器任务
    void TimerAdd(uint64_t id, task_t task, int time)
    {
        _timerwheel.TimerAdd(id, task, time);
    }

    // 刷新定时任务
    void TimerReFresh(uint64_t id)
    {
        _timerwheel.TimerReFresh(id);
    }

    // 取消定时任务
    void TimerCancel(uint64_t id)
    {
        _timerwheel.TimerCancel(id);
    }

    // 是否存在定时任务
    bool HasTimer(uint64_t id)
    {
        return _timerwheel.HasTimer(id);
    }
};

// 将线程和EventLoop一一对应
class LoopThread
{
private:
    EventLoop *_loop;
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;

private:
    void Entry()
    {
        EventLoop loop;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loop = &loop;
            _cond.notify_all();
        }
        loop.Start();
    }

public:
    LoopThread()
        : _thread(std::thread(&LoopThread::Entry, this)),
          _loop(nullptr)

    {
    }

    EventLoop *GetLoop()
    {
        EventLoop *loop = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock, [&]()
                       { return _loop != nullptr; });
            loop = _loop;
        }
        return loop;
    }
};

// 线程池
class LoopThreadPool
{
private:
    int _thread_count;                  // 线程池中从属EventLoop(线程)数量
    int _next_idx;                      // 获取EventLoop*的规则：采用RR轮转
    EventLoop *_base_loop;              // 主EventLoop
    std::vector<LoopThread *> _threads; // 从EventLoop
    std::vector<EventLoop *> _loops;    // 获取从EventLoop*的容器
public:
    LoopThreadPool(EventLoop *baseloop)
        : _thread_count(0), _next_idx(0), _base_loop(baseloop)
    {
    }
    void SetThreadCount(int threadcount)
    {
        _thread_count = threadcount;
    }
    EventLoop *NextLoop()
    {
        // RR轮转
        if (_thread_count == 0)
            return _base_loop;
        _next_idx = (_next_idx + 1) % _thread_count;
        return _loops[_next_idx];
    }
    void Create()
    {
        if (_thread_count > 0)
        {
            _threads.resize(_thread_count);
            _loops.resize(_thread_count);
            for (int i = 0; i < _thread_count; ++i)
            {
                _threads[i] = new LoopThread();
                // 这里一定不会有线程安全问题，因为LoopThread采用了 互斥锁+条件变量 来保证EventLoop获取的同步
                _loops[i] = _threads[i]->GetLoop();
            }
        }
    }
};

class Any
{
    class holder
    {
    public:
        holder() {}
        virtual ~holder() {}
        virtual const std::type_info &Type() = 0;
        virtual holder *Clone() = 0;

    private:
    };
    template <typename T>
    class place_holder : public holder
    {
    public:
        place_holder(const T &val) : _val(val) {}
        virtual ~place_holder() {}
        virtual const std::type_info &Type()
        {
            assert(typeid(T) == typeid(_val));
            return typeid(_val);
        }
        virtual holder *Clone()
        {
            return new place_holder(_val);
        }

    public:
        T _val;
    };

public:
    Any() : _hold(nullptr) {}

    ~Any()
    {
        if (_hold)
        {
            delete _hold;
            _hold = nullptr;
        }
    }

    template <class T>
    Any(const T &t) : _hold(new place_holder<T>(t)) {}
    Any(const Any &other) : _hold(other._hold ? other._hold->Clone() : nullptr) {}
    template <class T>
    Any &operator=(const T &t)
    {
        Any temp(t);
        Swap(temp);
        return *this;
    }
    Any &operator=(const Any &other)
    {
        Any temp(other);
        Swap(temp);
        return *this;
    }
    template <class T>
    T *Get()
    {
        return &((reinterpret_cast<place_holder<T> *>(_hold))->_val);
    }

    void Swap(Any &other)
    {
        std::swap(_hold, other._hold);
    }

private:
    holder *_hold;
};

class Connection;
enum Status
{
    CONNECTED,    // 连接状态
    CONNECTING,   // 半连接状态
    DISCONNECTED, // 关闭状态
    DISCONNECTING // 半关闭状态
};
using PtrConnection = std::shared_ptr<Connection>;
class Connection : public std::enable_shared_from_this<Connection>
{
private:
    uint64_t _con_id;              // 连接id
    int _sockfd;                   // 文件描述符
    bool _enable_inactive_destroy; // 是否进行非活跃连接销毁标识符
    Status _status;
    EventLoop *_loop;   // 让连接和线程一一对应
    Socket _socket;     // 管理文件描述符的操作
    Channel _channel;   // 对连接事件的管理
    Buffer _in_buffer;  // 输入缓冲区
    Buffer _out_buffer; // 输出缓冲区
    Any _context;       // 协议上下文数据
    using ConnectCallback = std::function<void(const PtrConnection &)>;
    using MessageCallback = std::function<void(const PtrConnection &, Buffer *)>;
    using CloseCallback = std::function<void(const PtrConnection &)>;
    using AnyEventCallback = std::function<void(const PtrConnection &)>;
    // 回调函数管理
    ConnectCallback _connect_callback;
    MessageCallback _message_callback;
    CloseCallback _close_callback;
    CloseCallback _server_close_callback;
    AnyEventCallback _anyevent_callback;

private:
    // 五个channel回调函数
    // 描述符触发读事件
    void HandleRead()
    {
        // 1.从套接字读取数据 2.业务处理
        char buffer[65535] = {0};
        ssize_t ret = _socket.RecvNonBlock(buffer, sizeof(buffer) - 1);
        // DEBUG_LOG("sockfd: %d", _socket.Fd());

        if (ret < 0)
        {
            return ShutdownInLoop();
        }
        _in_buffer.WriteAndPush(buffer, ret);
        if (_in_buffer.ReadAbleSize() > 0)
        {
            _message_callback(shared_from_this(), &_in_buffer);
        }
    }
    // 描述符触发写事件
    void HandleWrite()
    {
        ssize_t ret = _socket.SendNonBlock(_out_buffer.ReadPosition(), _out_buffer.ReadAbleSize());
        // DEBUG_LOG("sockfd: %d", _socket.Fd());
        if (ret < 0)
        {
            // 写出错了，如果输入缓冲区中有数据就先就进行业务处理再关闭连接
            if (_in_buffer.ReadAbleSize() > 0)
            {
                _message_callback(shared_from_this(), &_in_buffer);
            }
            return Release();
        }
        _out_buffer.MoveReadBack(ret);
        // 如果输出缓冲区中没有数据了,关闭写监控
        if (_out_buffer.ReadAbleSize() == 0)
        {
            _channel.DisableWrite();
            // 如果处于关闭中的状态
            if (_status == DISCONNECTING)
            {
                
                return Release();
            }
        }
    }
    // 描述符触发挂断事件
    void HandleClose()
    {
        // 此时描述符已经不可用了，有数据要处理就处理一下
        if (_in_buffer.ReadAbleSize() > 0)
        {
            _message_callback(shared_from_this(), &_in_buffer);
        }

        return Release();
    }
    // 描述符触发错误事件
    void HandleError()
    {
        return HandleClose();
    }
    // 描述符触发任意事件
    void HandleEvent()
    {
        // 如果组件设置了任意回调则调用一下
        if (_anyevent_callback)
            _anyevent_callback(shared_from_this());
        // 如果启动了非活跃销毁机制
        if (_enable_inactive_destroy == true)
            _loop->TimerReFresh(_con_id);
    }
    // 连接获取之后，所处的状态下要进行的操作(启动读监控， 调用回调函数)
    void EstablishedInLoop()
    {
        assert(_status == CONNECTING);
        _status = CONNECTED;
        _channel.EnableRead();
        if (_connect_callback)
            _connect_callback(shared_from_this());
    }
    // 真正意义的释放连接
    void ReleaseInLoop()
    {
        // 1.取消对连接事件的监听
        _loop->RemoveEvent(&_channel);
        // 2.如果定时器中还有该连接非活跃连接的销毁,需要删掉避免多次释放该连接
        if (_loop->HasTimer(_con_id) == true)
        {
            CancelInactiveDestroyInLoop();
        }
        // 3.关闭文件描述符
        _socket.Close();
        // 4.如果上层设置了回调函数就调用,必须先调用，避免server内的连接信息被删除，导致连接被释放，造成野指针问题
        if (_close_callback)
            _close_callback(shared_from_this());
        // 5.移除服务器内部管理的连接信息
        if (_server_close_callback)
            _server_close_callback(shared_from_this());
    }
    // 发送数据, 1.把数据拷贝到输出缓冲区 2.监控写事件
    void SendInLoop(Buffer &buffer)
    {
        if (_status == DISCONNECTED)
            return;
        _out_buffer.WriteBufferAndPush(buffer);
        if (_channel.WriteAble() == false)
        {
            _channel.EnableWrite();
        }
    }
    // 关闭连接,并不是真正意义的关闭，首先要检查输入输出缓冲区是否有数据
    void ShutdownInLoop()
    {
        _status = DISCONNECTING;
        // 如果输入缓冲区中有数据处理一下
        if (_in_buffer.ReadAbleSize() > 0)
        {
            _message_callback(shared_from_this(), &_in_buffer);
        }
        // 1.要么就是写入的时候出错释放，要么就是输出缓冲区中没有数据了直接释放
        // 2.不需要考虑输入缓冲区中是否有未处理的数据，因为它可能永远都不完整了，无法处理
        if (_out_buffer.ReadAbleSize() > 0)
        {
            if (_channel.WriteAble() == false)
            {
                _channel.EnableWrite();
            }
        }
        if (_out_buffer.ReadAbleSize() == 0)
        {
            Release();
        }
    }
    // 开启非活跃连接,如果已经开启了就刷新
    void EnableInactiveDestroyInLoop(int second)
    {
        // 设置标记位
        _enable_inactive_destroy = true;

        if (_loop->HasTimer(_con_id))
            _loop->TimerReFresh(_con_id);
        else
            _loop->TimerAdd(_con_id, std::bind(&Connection::HandleClose, this), second);
    }
    // 关闭非活跃连接
    void CancelInactiveDestroyInLoop()
    {
        // 设置标记位 如果存在定时任务就删除
        _enable_inactive_destroy = false;
        if (_loop->HasTimer(_con_id))
        {
            _loop->TimerCancel(_con_id);
        }
    }

    void UpgradeInLoop(const Any &context, const ConnectCallback &con,
                       const MessageCallback &mes,
                       const CloseCallback &clo,
                       const AnyEventCallback &eve)
    {
        _context = context;
        _connect_callback = con;
        _message_callback = mes;
        _close_callback = clo;
        _anyevent_callback = eve;
    }

public:
    Connection(uint64_t id, int sockfd, EventLoop *loop)
        : _con_id(id), _sockfd(sockfd),
          _enable_inactive_destroy(false),
          _status(CONNECTING), _loop(loop),
          _socket(sockfd), _channel(sockfd, loop)
    {
        _channel.SetReadCallBack(std::bind(&Connection::HandleRead, this));
        _channel.SetWriteCallBack(std::bind(&Connection::HandleWrite, this));
        _channel.SetCloseCallBack(std::bind(&Connection::HandleClose, this));
        _channel.SetErrCallBack(std::bind(&Connection::HandleError, this));
        _channel.SetEventCallBack(std ::bind(&Connection::HandleEvent, this));
    }
    ~Connection() {};
    uint64_t GetId()
    {
        return _con_id;
    }
    int GetFd()
    {
        return _sockfd;
    }
    void SetContext(const Any &context)
    {
        _context = context;
    }
    Any *GetContext()
    {
        return &_context;
    }
    // 判断是否处于连接状态
    bool IsConnected()
    {
        return _status == CONNECTED;
    }
    void SetConnectCallback(const ConnectCallback &cb)
    {
        _connect_callback = cb;
    }
    void SetMessageCallback(const MessageCallback &cb)
    {
        _message_callback = cb;
    }
    void SetCloseCallback(const CloseCallback &cb)
    {
        _close_callback = cb;
    }
    void SetServerCloseCallback(const CloseCallback &cb)
    {
        _server_close_callback = cb;
    }
    void SetAnyEventCallback(const AnyEventCallback &cb)
    {
        _anyevent_callback = cb;
    }
    // 连接获取之后，所处的状态下要进行的操作(启动读监控， 调用回调函数)
    void Established()
    {
        return _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop, this));
    }

    // 发送数据, 1.把数据拷贝到输出缓冲区 2.监控输出
    void Send(const char *buf, size_t len)
    {
        // 防止buf释放，自己构造一个Buffer出来
        Buffer buffer;
        buffer.WriteAndPush(buf, len);
        return _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, std::move(buffer)));
    }
    // 关闭连接,并不是真正意义的关闭，首先要检查输入输出缓冲区是否有数据
    void Shutdown()
    {
        return _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop, this));
    }
    void Release()
    {
        // 如果向任务池中压入了一次释放任务就不要再压入了
        if(_status == DISCONNECTED) return;
        // 标志已经压入任务池
        _status = DISCONNECTED;
        return _loop->QueueInLoop(std::bind(&Connection::ReleaseInLoop, this));
    }
    // 开启非活跃连接,如果已经开启了就刷新
    void EnableInactiveDestroy(int second)
    {
        return _loop->RunInLoop(std::bind(&Connection::EnableInactiveDestroyInLoop, this, second));
    }
    // 关闭非活跃连接
    void CancelInactiveDestroy()
    {
        return _loop->RunInLoop(std::bind(&Connection::CancelInactiveDestroyInLoop, this));
    }

    // 切换协议---重置上下文以及阶段性回调处理函数 -- 而是这个接口必须在EventLoop线程中立即执行
    // 防备新的事件触发后，处理的时候，切换任务还没有被执行--会导致数据使用原协议处理了。
    void Upgrade(const Any &context, const ConnectCallback &con,
                 const MessageCallback &mes,
                 const CloseCallback &clo,
                 const AnyEventCallback &eve)
    {
        _loop->AssertInLoop();
        return _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop, this, context, con, mes, clo, eve));
    }
};

// 连接触发器
class Accepter
{
private:
    Socket _socket; // 监听套接字的管理
    EventLoop *_loop;
    Channel _channel; // 监听事件的管理
    using AccpterCallback = std::function<void(int)>;
    AccpterCallback _accepter_callback; // 对于新获取的套接字的处理，由服务器模块设置
private:
    int CreateServer(uint16_t port)
    {
        bool ret = _socket.CreateServer(port);
        assert(ret);
        return _socket.Fd();
    }
    void HandlerRead()
    {
        // 1.获取套接字  2.调用回调函数
        int newsock = _socket.Accept();
        if (newsock < 0)
            return;
        if (_accepter_callback)
            _accepter_callback(newsock);
    }

public:
    // 构造函数不能直接启动读事件监听，因为还没有设置回调函数的话，触发读事件后没法处理，套接字也不会关闭,造成资源泄漏
    Accepter(uint16_t port, EventLoop *loop)
        : _socket(CreateServer(port)), _loop(loop), _channel(_socket.Fd(), loop)
    {
        _channel.SetReadCallBack(std::bind(&Accepter::HandlerRead, this));
    }
    void SetAccpterCallback(const AccpterCallback &cb)
    {
        _accepter_callback = cb;
    }
    void Listen()
    {
        _channel.EnableRead();
    }
};

// 服务器组件
class TcpServer
{
private:
    int _port;                                          // 监听端口
    uint64_t _next_id;                                  // 处理连接id和定时任务id
    int _thread_count;                                  // 从属线程数量
    bool _enable_inactive_destroy;                      // 是否开启非活跃连接销毁机制
    int _timeout;                                       // 非活跃连接时长
    EventLoop *_base_loop;                              // 主EventLoop
    LoopThreadPool _loops;                              // 线程池
    Accepter _accepter;                                 // 连接管理器
    std::unordered_map<uint64_t, PtrConnection> _conns; // 管理最后一份PtrConnection
    using Functor = std::function<void()>;
    using ConnectCallback = std::function<void(const PtrConnection &)>;
    using MessageCallback = std::function<void(const PtrConnection &, Buffer *)>;
    using CloseCallback = std::function<void(const PtrConnection &)>;
    using AnyEventCallback = std::function<void(const PtrConnection &)>;
    // 回调函数管理
    ConnectCallback _connect_callback;
    MessageCallback _message_callback;
    CloseCallback _close_callback;
    AnyEventCallback _anyevent_callback;

private:
    void NewConnection(int newsock)
    {
        _next_id++;
        PtrConnection conn(new Connection(_next_id, newsock, _loops.NextLoop()));
        conn->SetConnectCallback(_connect_callback);
        conn->SetMessageCallback(_message_callback);
        conn->SetCloseCallback(_close_callback);
        conn->SetAnyEventCallback(_anyevent_callback);
        conn->SetServerCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
        // 判断是否启动了超时销毁机制
        if (_enable_inactive_destroy == true)
            conn->EnableInactiveDestroy(_timeout);
        conn->Established();
        _conns.insert(make_pair(_next_id, conn));
    }
    // 服务器模块释放连接
    void RemoveConnectionInLoop(const PtrConnection &conn)
    {
        uint64_t id = conn->GetId();
        _conns.erase(id);
    }
    // 服务器模块释放连接
    void RemoveConnection(const PtrConnection &conn)
    {
        // 服务器释放叫给主线程，避免同时释放，红黑树结构被破坏,造成线程安全问题
        return _base_loop->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
    }
    // 添加定时任务
    void TimerAddInLoop(const Functor &task, int delay)
    {
        _next_id++;
        _base_loop->TimerAdd(_next_id, task, delay);
    }

public:
    TcpServer(int port)
        : _port(port), _next_id(0), _thread_count(0),
          _enable_inactive_destroy(false), _timeout(0),
          _base_loop(new EventLoop()), _loops(_base_loop),
          _accepter(port, _base_loop)
    {
        _accepter.SetAccpterCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
        _accepter.Listen();
    }
    // 设置从属线程数量
    void SetThreadCount(int count)
    {
        _thread_count = count;
        _loops.SetThreadCount(_thread_count);
    }
    // 开启非活跃连接销毁
    void EnableInactiveDestroy(int timeout)
    {
        _enable_inactive_destroy = true;
        _timeout = timeout;
    }
    // 添加定时任务
    void TimerAdd(const Functor &task, int delay)
    {
        _base_loop->RunInLoop(std::bind(&TcpServer::TimerAddInLoop, this, task, delay));
    }

    void SetConnectCallback(const ConnectCallback &cb)
    {
        _connect_callback = cb;
    }
    void SetMessageCallback(const MessageCallback &cb)
    {
        _message_callback = cb;
    }
    void SetCloseCallback(const CloseCallback &cb)
    {
        _close_callback = cb;
    }
    void SetAnyEventCallback(const AnyEventCallback &cb)
    {
        _anyevent_callback = cb;
    }
    void Start()
    {
        _loops.Create();
        _base_loop->Start();
    }
};

// 更新对事件的监控
void Channel::Update()
{
    _loop->UpdateEvent(this);
}
// 移除监控
void Channel::Remove()
{
    _loop->RemoveEvent(this);
}

// 添加任务的定时器,_id_timer_map的访问存在线程安全，如果不想加锁的话就放在一个线程内执行
void TimerWheel::TimerAdd(uint64_t id, task_t task, int time)
{
    _loop->RunInLoop(std::bind(&TimerWheel::TimerAddInLoop, this, id, task, time));
}
// 刷新活跃度
void TimerWheel::TimerReFresh(uint64_t id)
{
    _loop->RunInLoop(std::bind(&TimerWheel::TimerReFreshInLoop, this, id));
}
// 取消定时任务
void TimerWheel::TimerCancel(uint64_t id)
{
    _loop->RunInLoop(std::bind(&TimerWheel::TimerCancelInLoop, this, id));
}

class NetWork
{
public:
    NetWork()
    {
        // DEBUG_LOG("SIGPIPE IGNORE");
        signal(SIGPIPE, SIG_IGN);
    }
};
static NetWork net;