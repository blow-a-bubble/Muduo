/*超时连接测试1：创建一个客户端，给服务器发送一次数据后，不动了，查看服务器是否会正常的超时关闭连接*/
#include "../server.hpp"
int main()
{
    Socket sock;
    sock.CreateClient(8080, "127.0.0.1");
    std::string req = "GET / HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(1)
    {
        sleep(15);
        sock.Send(req.c_str(), req.size());
        char buffer[1024] = { 0 };
        sock.Recv(buffer, 1023);
        DEBUG_LOG("[%s]", buffer);
    }
    return 0;
}