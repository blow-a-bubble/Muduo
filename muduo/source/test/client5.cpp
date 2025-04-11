/*一次性给服务器发送多条数据，然后查看服务器的处理结果*/
/*每一条请求都应该得到正常处理*/
#include "../server.hpp"
int main()
{
    Socket sock;
    sock.CreateClient(8080, "127.0.0.1");
    std::string req = "PUT /123.txt HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(1)
    {
        sock.Send(req.c_str(), req.size());
        // sleep(1);
        sock.Send(req.c_str(), req.size());
        sock.Send(req.c_str(), req.size());

        char buffer[1024] = { 0 };
        sock.Recv(buffer, 1023);
        DEBUG_LOG("[%s]", buffer);
        sleep(3);

    }
    return 0;
}