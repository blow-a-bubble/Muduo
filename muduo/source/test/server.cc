#include "../server.hpp"
void OnConnect(const PtrConnection& con)
{
    DEBUG_LOG("get a new link: %p", con.get());
}
void OnMessage(const PtrConnection& con, Buffer* buffer)
{
    std::string response = buffer->ReadAsStringAndPop(buffer->ReadAbleSize());
    DEBUG_LOG("recv a message: %s", response.c_str());
    con->Send(&response[0], response.size());
    // con->Shutdown();
}
void OnClose(const PtrConnection& con)
{
    DEBUG_LOG("close a link: %p", con.get());
}
void DebugTimer()
{
    INFO_LOG("闹钟响了。。。");
}
int main()
{
    TcpServer svr(9999);
    svr.SetConnectCallback(OnConnect);
    svr.SetMessageCallback(OnMessage);
    svr.SetCloseCallback(OnClose);
    svr.SetThreadCount(2);
    svr.EnableInactiveDestroy(10);
    svr.TimerAdd(DebugTimer, 5);
    svr.Start();
    return 0;
}