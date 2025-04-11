/*长连接测试1：创建一个客户端持续给服务器发送数据，直到超过超时时间看看是否正常*/
#include "../server.hpp"

int main()
{
    Socket sock;
    sock.CreateClient(8080, "127.0.0.1");
    std::string req = "GET / HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(1)
    {
        sock.Send(req.c_str(), req.size());
        char buffer[1024] = { 0 };
        sock.Recv(buffer, 1023);
        std::cout << buffer << std::endl;
        sleep(3);
    }
    return 0;
}