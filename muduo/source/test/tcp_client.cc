#include "../server.hpp"

int main()
{
    Socket sock;
    sock.CreateClient(9999, "127.0.0.1");
    for(int i = 0; i < 5; ++i)
    {
        std::string str = "hello world";
        sock.Send(str.c_str(), str.size());
        char buffer[1024] = {0};
        sock.Recv(buffer, 1023);
        // std::cout << buffer << std::endl;
        INFO_LOG("%s", buffer);
        sleep(1);
    }
    while(1);
    sock.Close();
    return 0;
}