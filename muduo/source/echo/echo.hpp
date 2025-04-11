#include "../server.hpp"

class EchoServer
{
private:
    TcpServer _tcpsvr;
private:
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
public:
    EchoServer(int port)
    :_tcpsvr(port)
    {
        _tcpsvr.SetConnectCallback(std::bind(&EchoServer::OnConnect, this, std::placeholders::_1));
        _tcpsvr.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
        _tcpsvr.SetCloseCallback(std::bind(&EchoServer::OnClose, this, std::placeholders::_1));
        _tcpsvr.EnableInactiveDestroy(10);
        _tcpsvr.SetThreadCount(8);
    }
    void Start()
    {
        _tcpsvr.Start();
    }

};