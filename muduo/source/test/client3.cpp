/*给服务器发送一个数据，告诉服务器要发送1024字节的数据，但是实际发送的数据不足1024，查看服务器处理结果*/
/*
    1. 如果数据只发送一次，服务器将得不到完整请求，就不会进行业务处理，客户端也就得不到响应，最终超时关闭连接
    2. 连着给服务器发送了多次 小的请求，  服务器会将后边的请求当作前边请求的正文进行处理，而后便处理的时候有可能就会因为处理错误而关闭连接
*/
#include "../server.hpp"
int main()
{
    Socket sock;
    sock.CreateClient(8080, "127.0.0.1");
    std::string req = "PUT /123.txt HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 5\r\n\r\n";
    while(1)
    {
        sock.Send(req.c_str(), req.size());
        // sleep(1);
        sock.Send(req.c_str(), req.size());

        char buffer[1024] = { 0 };
        sock.Recv(buffer, 1023);
        DEBUG_LOG("[%s]", buffer);
        sleep(3);

    }
    return 0;
}
