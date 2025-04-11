#include "http.hpp"
std::string RequestStr(const HttpRequest &req) 
{
    std::stringstream ssm;
    ssm << req._method << " " << req._path << " " << req._version << "\r\n";
    for(const auto& e : req._param)
    {
        ssm << e.first << ": " << e.second << "\r\n";
    }
    for(const auto& e : req._headers)
    {
        ssm << e.first << ": " << e.second << "\r\n";
    }
    ssm << "\r\n";
    ssm << req._body;
    return ssm.str();
}
void Get(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

void Post(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

void Put(const HttpRequest& req, HttpResponse* resp)
{
    // resp->SetContent(RequestStr(req), "text/plain");
    std::string body = req._body;
    Util::WriteFile("./123.txt", body);
}

void Delete(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

int main()
{
    HttpServer svr(8080);
    svr.SetBaseDir("/home/blow/linux_code/Muduo/muduo/source/http/wwwroot");
    svr.SetThreadCount(2);
    svr.Get("/hello", Get);
    svr.Post("/login", Post);
    svr.Put("/123.txt", Put);
    svr.Delete("/123.txt", Delete);

    svr.Listen();

    /*
    std::cout << Util::IsDirectory("testdir") << std::endl;
    std::cout << Util::IsDirectory("main") << std::endl;
    std::cout << Util::IsRegular("testdir") << std::endl;
    std::cout << Util::IsRegular("main") << std::endl;
    std::cout << Util::IsValidPath("/a/../b.html") << std::endl;
    */
    /*
    std::cout << Util::StatusDesc(500) << std::endl;
    std::cout << Util::ExtMime("file.jpg") << std::endl;;
    */
    /*
    std::string url = "/index.html/c  ";
    std::string temp = Util::UrlEncode(url, false);
    std::cout << temp << std::endl;
    url = Util::UrlDecode(temp, false);
    std::cout << url << std::endl;
    */
    /*
    std::string filename = "http.hpp";
    std::string str;
    bool ret = Util::ReadFile(filename, &str);
    assert(ret);
    // std::cout << str << std::endl;
    ret = Util::WriteFile("aaa.txt", str);
    assert(ret);
    */
    /*
    std::string str = "/a/bc/d/";
    std::vector<std::string> ret;
    Util::Split(str, "/", &ret);
    for(auto & e: ret)
    {
        std::cout << "[" << e << "]" << std::endl;
    }
    */
    return 0;
}