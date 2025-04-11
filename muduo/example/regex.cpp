#include <iostream>
#include <string>
#include <regex>
using std::cout;
using std::endl;
int main()
{
    // std::string str("/abc/1234");
    // std::smatch matchs;
    // std::regex e("/abc/(\\d+)");
    // bool flag = std::regex_match(str, matchs, e);


    std::string str = "get /index.html?username=pwd&passwd=123456 HTTP/1.1\r\n";
    std::smatch matchs;
    // std::regex e(R"(GET|PUT|POST) ([^?]*)\?(.*) (HTTP/1\.[01])");
    std::regex e(R"((GET|PUT|POST) ([^?]*)(?:\?(.*))? (HTTP/1\.[01])(?:\r\n|\n))", std::regex::icase);
    // std::regex e(R"(^(GET|PUT|POST) ([^?]*)(\?.*)? (HTTP/1\.[01])$)");
    bool flag = std::regex_match(str, matchs, e);
    if(flag == false)
    {
        return -1;
    }
    std::string method = matchs[1];
    std::transform(method.begin(), method.end(), method.begin(), ::toupper);
    std::cout << method << std::endl;
    std::cout << matchs.size() << std::endl;
    for(auto& s : matchs)
    {
        cout << s << endl;
    }
    return 0;
}