#include "../server.hpp"
using namespace std;
// void CloseCallBack(Channel* channel)
// {

//     // cout << "关闭了一个sockfd: " << channel->Fd() << endl;
//     INFO_LOG("关闭了一个sockfd: %d", channel->Fd());
//     channel->Remove();

//     close(channel->Fd());
//     delete channel;
// }
// void ReadCallBack(Channel* channel)
// {
//     int fd = channel->Fd();
//     char buffer[1024] = {0};
//     ssize_t n = recv(fd, buffer, 1023, 0);
//     if(n > 0)
//     {
//         buffer[n] = 0;
//         // cout << buffer << endl;
//         INFO_LOG("%s", buffer);
//         channel->EnableWrite();
//     }
//     else
//     {
//         CloseCallBack(channel);
//     }
// }
// void WriteCallBack(Channel* channel)
// {
//     int fd = channel->Fd();
//     std::string str = "hello mutuo";
//     ssize_t n = send(fd, str.c_str(), str.size(), 0);
//     if(n < 0)
//     {
//         CloseCallBack(channel);
//     }
//     channel->DisableWrite();
// }

// void ErrCallBack(Channel* channel)
// {
//     CloseCallBack(channel);
// }
// void EventCallBack(EventLoop* loop, Channel* channel, uint64_t timerid)
// {
//     // cout << "触发了一个事件" << endl;
//     loop->TimerReFresh(timerid);
// }
// void Accepter(EventLoop* loop, Channel* listen)
// {
//     int fd = listen->Fd();
//     //1.accept
//     int newsock = accept(fd, nullptr, nullptr);
//     uint64_t timerid = rand() % 10000;
//     //2.为新连接创建channel
//     Channel* channel = new Channel(newsock, loop);
//     //3.设置回调
//     channel->SetReadCallBack(std::bind(ReadCallBack, channel));
//     channel->SetWriteCallBack(std::bind(WriteCallBack, channel));
//     channel->SetErrCallBack(std::bind(ErrCallBack, channel));
//     channel->SetCloseCallBack(std::bind(CloseCallBack, channel));
//     channel->SetEventCallBack(std::bind(EventCallBack, loop, channel, timerid));

//     //必须在启动监听事件之前，因为启动监听了，可能立马有事件就绪
//     loop->TimerAdd(timerid, std::bind(CloseCallBack, channel), 10);
//     //4.添加读事件监听
//     channel->EnableRead();
// }

// int main()
// {
//     // Poller poller;
//     srand(time(nullptr));
//     EventLoop loop;
//     Socket listen;
//     listen.CreateServer(9999);
//     Channel* channel = new Channel(listen.Fd(), &loop);
//     channel->SetReadCallBack(std::bind(Accepter, &loop, channel));
//     channel->EnableRead();
//     while(true)
//     {
//         loop.Start();
//     }
//     return 0;
// }



std::unordered_map<uint64_t, PtrConnection> conns;
uint64_t id = 0;
EventLoop baseloop;
// std::vector<LoopThread> loops(2);
LoopThreadPool loops(&baseloop);
// int idx = 0;
void OnConnect(const PtrConnection& con)
{
    DEBUG_LOG("get a new link: %p", con.get());
}
void OnMessage(const PtrConnection& con, Buffer* buffer)
{
    std::string response = buffer->ReadAsStringAndPop(buffer->ReadAbleSize());
    DEBUG_LOG("recv a message: %s", response.c_str());
    con->Send(&response[0], response.size());
    con->Shutdown();
}
void OnClose(const PtrConnection& con)
{
    DEBUG_LOG("close a link: %p", con.get());
}
void OnServerClose(const PtrConnection& con)
{
    uint64_t id = con->GetId();
    conns.erase(id);
}
void NewConnection(int newsock)
{
    // idx = (idx + 1) % 2;
    //2.为新连接创建channel
    // PtrConnection conn(new Connection(id, newsock, loops[idx].GetLoop()));
    PtrConnection conn(new Connection(id, newsock, loops.NextLoop()));

    //3.设置回调
    conn->SetConnectCallback(std::bind(OnConnect, std::placeholders::_1));
    conn->SetMessageCallback(std::bind(OnMessage, std::placeholders::_1, std::placeholders::_2));
    conn->SetCloseCallback(std::bind(OnClose, std::placeholders::_1));
    conn->SetServerCloseCallback(std::bind(OnServerClose, std::placeholders::_1));
    conn->EnableInactiveDestroy(10);
    conn->Established();
    conns.insert(make_pair(id, conn));
    id++;
    DEBUG_LOG("new connection---------");
}

int main()
{
    // Poller poller;
    srand(time(nullptr));
    // Socket listen;
    
    // listen.CreateServer(9999);
    // Channel* channel = new Channel(listen.Fd(), &loop);
    // channel->SetReadCallBack(std::bind(Accepter, &loop, channel));
    // channel->EnableRead();
    loops.SetThreadCount(2);
    loops.Create();
    Accepter accepter(9999, &baseloop);
    accepter.SetAccpterCallback(NewConnection);
    accepter.Listen();
    baseloop.Start();
    return 0;
}