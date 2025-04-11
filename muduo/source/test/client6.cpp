/*大文件传输测试，给服务器上传一个大文件，服务器将文件保存下来，观察处理结果*/
/*
    上传的文件，和服务器保存的文件一致
*/

#include "../http/http.hpp"

int main()
{
    Socket client;
    client.CreateClient(8080, "127.0.0.1");
    std::string req = "PUT /123.txt HTTP/1.1\r\n";
    std::string body;
    Util::ReadFile("largefile", &body);
    req += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    client.Send(req.c_str(), req.size());
    client.Send(body.c_str(), body.size());
    char buffer[1024] = {0};
    std::cout << buffer << std::endl;
    return 0;
}