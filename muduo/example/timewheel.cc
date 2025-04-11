#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>
#include <unistd.h>

using task_t = std::function<void()>;
using release_t = std::function<void()>;
// 对事件进行封装
class TimerTask : public std::enable_shared_from_this<TimerTask>
{
private:
    uint64_t _id;          // 事件id
    task_t _task_cb;       // 任务回调
    int _time;             // 定时器时长
    bool _run;             // 是否启动事件定时器
    release_t _release_cb; // 定时器触发后的释放回调
public:
    TimerTask(uint64_t id, task_t task_cb, int time)
        : _id(id), _task_cb(task_cb), _run(true), _time(time) {}
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
public:
    TimerWheel(int capacity)
        : _capacity(capacity), _wheel(capacity), _step(0)
    {
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
    // 添加任务的定时器
    void TimerAdd(uint64_t id, task_t task, int time)
    {
        //定时器不能超过时间轮范围
        if(time < 0 || time > _capacity) return;
        PtrTask timer(new TimerTask(id, task, time));
        timer->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
        int pos = (_step + time) % _capacity;
        _wheel[pos].push_back(timer);
        _id_timer_map[id] = WeakTask(timer);
    }

    void TimerReFresh(uint64_t id)
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
        int time = timer->GetDelay();
        int pos = (_step + time) % _capacity;
        _wheel[pos].push_back(timer);
    }

    void TimerCancel(uint64_t id)
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

    // 运行时间轮
    void RunTimerWheel()
    {
        std::cout << "--------" << std::endl;
        sleep(1);
        _step++;
        _step %= _capacity;
        _wheel[_step].clear();
    }
};

class Test
{
public:
    Test() { std::cout << "构造" << std::endl; }
    ~Test() { std::cout << "析构" << std::endl; }
};
void Free(Test *t)
{
    delete t;
}
int main()
{
    Test *t = new Test();
    TimerWheel tw(30);
    tw.TimerAdd(999, std::bind(Free, t), 5);
    for (int i = 0; i < 5; ++i)
    {
        sleep(1);
        tw.TimerReFresh(999);
        tw.RunTimerWheel();
        std::cout << "刷新了一下定时任务，重新需要5s后才会销毁\n";
    }
    // tw.TimerCancel(999);
    std::cout << "取消了定时任务\n";
    while (true)
    {
        sleep(1);
        tw.RunTimerWheel();
        std::cout << "-------\n";
    }
    return 0;
}