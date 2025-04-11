#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <ctype.h>
#include <fstream>
#include <sys/stat.h>
#include <regex>
#include <sstream>
#include "../server.hpp"
#define DEFAULT_TIMEOUT 10
std::unordered_map<int, std::string> _statu_msg = {
    {100,  "Continue"},
    {101,  "Switching Protocol"},
    {102,  "Processing"},
    {103,  "Early Hints"},
    {200,  "OK"},
    {201,  "Created"},
    {202,  "Accepted"},
    {203,  "Non-Authoritative Information"},
    {204,  "No Content"},
    {205,  "Reset Content"},
    {206,  "Partial Content"},
    {207,  "Multi-Status"},
    {208,  "Already Reported"},
    {226,  "IM Used"},
    {300,  "Multiple Choice"},
    {301,  "Moved Permanently"},
    {302,  "Found"},
    {303,  "See Other"},
    {304,  "Not Modified"},
    {305,  "Use Proxy"},
    {306,  "unused"},
    {307,  "Temporary Redirect"},
    {308,  "Permanent Redirect"},
    {400,  "Bad Request"},
    {401,  "Unauthorized"},
    {402,  "Payment Required"},
    {403,  "Forbidden"},
    {404,  "Not Found"},
    {405,  "Method Not Allowed"},
    {406,  "Not Acceptable"},
    {407,  "Proxy Authentication Required"},
    {408,  "Request Timeout"},
    {409,  "Conflict"},
    {410,  "Gone"},
    {411,  "Length Required"},
    {412,  "Precondition Failed"},
    {413,  "Payload Too Large"},
    {414,  "URI Too Long"},
    {415,  "Unsupported Media Type"},
    {416,  "Range Not Satisfiable"},
    {417,  "Expectation Failed"},
    {418,  "I'm a teapot"},
    {421,  "Misdirected Request"},
    {422,  "Unprocessable Entity"},
    {423,  "Locked"},
    {424,  "Failed Dependency"},
    {425,  "Too Early"},
    {426,  "Upgrade Required"},
    {428,  "Precondition Required"},
    {429,  "Too Many Requests"},
    {431,  "Request Header Fields Too Large"},
    {451,  "Unavailable For Legal Reasons"},
    {501,  "Not Implemented"},
    {502,  "Bad Gateway"},
    {503,  "Service Unavailable"},
    {504,  "Gateway Timeout"},
    {505,  "HTTP Version Not Supported"},
    {506,  "Variant Also Negotiates"},
    {507,  "Insufficient Storage"},
    {508,  "Loop Detected"},
    {510,  "Not Extended"},
    {511,  "Network Authentication Required"}
};

std::unordered_map<std::string, std::string> _mime_msg = {
    {".aac",        "audio/aac"},
    {".abw",        "application/x-abiword"},
    {".arc",        "application/x-freearc"},
    {".avi",        "video/x-msvideo"},
    {".azw",        "application/vnd.amazon.ebook"},
    {".bin",        "application/octet-stream"},
    {".bmp",        "image/bmp"},
    {".bz",         "application/x-bzip"},
    {".bz2",        "application/x-bzip2"},
    {".csh",        "application/x-csh"},
    {".css",        "text/css"},
    {".csv",        "text/csv"},
    {".doc",        "application/msword"},
    {".docx",       "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot",        "application/vnd.ms-fontobject"},
    {".epub",       "application/epub+zip"},
    {".gif",        "image/gif"},
    {".htm",        "text/html"},
    {".html",       "text/html"},
    {".ico",        "image/vnd.microsoft.icon"},
    {".ics",        "text/calendar"},
    {".jar",        "application/java-archive"},
    {".jpeg",       "image/jpeg"},
    {".jpg",        "image/jpeg"},
    {".js",         "text/javascript"},
    {".json",       "application/json"},
    {".jsonld",     "application/ld+json"},
    {".mid",        "audio/midi"},
    {".midi",       "audio/x-midi"},
    {".mjs",        "text/javascript"},
    {".mp3",        "audio/mpeg"},
    {".mpeg",       "video/mpeg"},
    {".mpkg",       "application/vnd.apple.installer+xml"},
    {".odp",        "application/vnd.oasis.opendocument.presentation"},
    {".ods",        "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt",        "application/vnd.oasis.opendocument.text"},
    {".oga",        "audio/ogg"},
    {".ogv",        "video/ogg"},
    {".ogx",        "application/ogg"},
    {".otf",        "font/otf"},
    {".png",        "image/png"},
    {".pdf",        "application/pdf"},
    {".ppt",        "application/vnd.ms-powerpoint"},
    {".pptx",       "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar",        "application/x-rar-compressed"},
    {".rtf",        "application/rtf"},
    {".sh",         "application/x-sh"},
    {".svg",        "image/svg+xml"},
    {".swf",        "application/x-shockwave-flash"},
    {".tar",        "application/x-tar"},
    {".tif",        "image/tiff"},
    {".tiff",       "image/tiff"},
    {".ttf",        "font/ttf"},
    {".txt",        "text/plain"},
    {".vsd",        "application/vnd.visio"},
    {".wav",        "audio/wav"},
    {".weba",       "audio/webm"},
    {".webm",       "video/webm"},
    {".webp",       "image/webp"},
    {".woff",       "font/woff"},
    {".woff2",      "font/woff2"},
    {".xhtml",      "application/xhtml+xml"},
    {".xls",        "application/vnd.ms-excel"},
    {".xlsx",       "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml",        "application/xml"},
    {".xul",        "application/vnd.mozilla.xul+xml"},
    {".zip",        "application/zip"},
    {".3gp",        "video/3gpp"},
    {".3g2",        "video/3gpp2"},
    {".7z",         "application/x-7z-compressed"}
};

//http工具类
class Util
{
public:
    //根据sep分割字符串src，并把分割后的结果放到dest
    static size_t Split(const std::string &src, const std::string &sep, std::vector<std::string> *dest)
    {
        int offset = 0;  //偏移量：从哪开始找
        while(offset < src.size())
        {
            size_t pos = src.find(sep, offset);
            if(pos == std::string::npos)
            {
                dest->push_back(src.substr(offset));
                break;
            }
            
            if(pos != offset)
            {
                dest->push_back(src.substr(offset, pos - offset));
            }
            offset = pos + sep.size();
        }
        return dest->size();
    }
    //读取filename文件内容放到dest中
    static bool ReadFile(const std::string &filename, std::string* dest)
    {
        std::ifstream ifs(filename, std::ios::binary);
        if(ifs.is_open() == false)
        {
            printf("open file %s failed", filename.c_str());
            return false;
        }
        ifs.seekg(0, ifs.end);
        size_t size = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        dest->resize(size);
        ifs.read(&(*dest)[0], size);
        if(ifs.good() == false)
        {
            printf("read file %s failed", filename.c_str());
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }   
    //把src写到文件filename中
    static bool WriteFile(const std::string &filename, const std::string &src)
    {
        std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
        if(ofs.is_open() == false)
        {
            printf("open file %s failed", filename.c_str());
            return false;
        }
        ofs.write(&src[0], src.size());
        if(ofs.good() == false)
        {
            printf("write file %s failed", filename.c_str());
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }

    //不编码的特殊字符： RFC3986文档规定 . - _ ~ 字母，数字属于绝对不编码字符
    //W3C标准中规定，查询字符串中的空格，需要编码为+， 解码则是+转空格
    //url编码
    static std::string UrlEncode(const std::string &url, bool space_to_plus)
    {
        std::string ret;
        for(auto c : url)
        {
            if(isalnum(c) || c == '.' || c == '-' || c == '_' || c == '~')
            {
                ret += c;
                continue;
            }
            if(c == ' ' && space_to_plus == true)
            {
                ret += '+';
                continue;
            }
            char buffer[4] = {0};
            snprintf(buffer, 4, "%%%0X", c);
            ret += buffer;
        }
        return ret;
    }
    static char HexToI(char c)
    {
        if(c >= '0' && c <= '9')
        {
            return c - '0';
        }
        else if(c >= 'a' && c <= 'z')
        {
            return c - 'a' + 10;
        }
        else if(c >= 'A' && c <= 'Z')
        {
            return c - 'A' + 10;
        }
        else
        {
            return -1;
        }
        
    }
    //url解码
    static std::string UrlDecode(const std::string &url, bool space_to_plus)
    {
        std::string ret;
        for(int i = 0; i < url.size(); ++i)
        {
            if(url[i] == '+' && space_to_plus == true)
            {
                ret += ' ';
                continue;
            }
            if(url[i] == '%' && i + 2 < url.size())
            {
                char val1 = HexToI(url[i + 1]);
                char val2 = HexToI(url[i + 2]);
                char temp = (val1 << 4) + val2;
                ret += temp;
                i += 2;
                continue;
            }
            ret += url[i];
        }
        return ret;
    }
    //根据状态码找描述
    static std::string StatusDesc(int status)
    {
        auto it = _statu_msg.find(status);
        if(it == _statu_msg.end())
        {
            return "Unknown";
        }
        return it->second;
    }
    //根据文件名找Content-Type
    static std::string ExtMime(const std::string& filename)
    {
        auto it = filename.rfind(".");
        if(it == std::string::npos)
        {
            return "application/octet-stream";
        }
        std::string ext = filename.substr(it);
        auto ret = _mime_msg.find(ext);
        if(ret == _mime_msg.end())
        {
            return "application/octet-stream";
        }
        return ret->second;
    }
    //是否是目录
    static bool IsDirectory(const std::string &filename)
    {
        struct stat mode;
        int n = stat(filename.c_str(), &mode);
        if(n < 0) return false;
        return S_ISDIR(mode.st_mode);
    }
    //是否是普通文件
    static bool IsRegular(const std::string &filename)
    {
        struct stat mode;
        int n = stat(filename.c_str(), &mode);
        if(n < 0) return false;
        return S_ISREG(mode.st_mode);
    }
    //判断路径有效性
    static bool IsValidPath(const std::string &path)
    {
        std::vector<std::string> arry;
        Split(path, "/", &arry);
        int level = 0; //判断相对根目录几层,若小于0则不合法
        for(auto& e : arry)
        {
            if(e == "..")
            {
                level--;
                if(level < 0)
                {
                    return false;
                }
            }
            else
            {
                level++;
            }
        }
        return true;
    }
};

class HttpRequest
{
public:
    std::string _method;    //请求方法
    std::string _path;      //请求资源路径
    std::string _version;   //请求版本
    std::unordered_map<std::string, std::string> _param;  //查询字符串
    std::unordered_map<std::string, std::string> _headers; //请求报头
    std::string _body;   //请求正文
    std::smatch _smatch;    //保存首行经过regex正则库提取后，所提取的数据
public:
    HttpRequest()
    {
        _version = "HTTP/1.1";
    }
    //重置
    void Reset()
    {
        _method.clear();
        _path.clear();
        _version = "HTTP/1.1";
        _param.clear();
        _headers.clear();
        _body.clear();
        std::smatch temp;
        _smatch.swap(temp);
    }
    //查询字符串的插入，查询，获取
    void SetParam(const std::string &key, const std::string &val)
    {
        _param.insert(std::make_pair(key, val));
    }
    bool HasParam(const std::string &key) const
    {
        auto it = _param.find(key);
        if(it == _param.end())
        {
            return false;
        }
        return true;
    }
    std::string GetParam(const std::string &key) const
    {
        auto it = _param.find(key);
        if(it == _param.end())
        {
            return "";
        }
        return it->second;
    }
    //报头的插入，查询，获取
    void SetHeader(const std::string &key, const std::string &val)
    {
        _headers.insert(std::make_pair(key, val));
    }
    bool HasHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if(it == _headers.end())
        {
            return false;
        }
        return true;
    }
    std::string GetHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if(it == _headers.end())
        {
            return "";
        }
        return it->second;
    }
    //获取正文长度
    size_t BodySize() const
    {
        auto it = _headers.find("Content-Length");
        if(it == _headers.end())
        {
            return 0;
        }
        std::string len_str = it->second;
        return std::stoi(len_str);
    }
    //判段是否是短连接
    bool Close() const
    {
        if(HasHeader("Connection") && GetHeader("Connection") == "keep-alive")
        {
            return false;
        }
        return true;
    }

};

class HttpResponse
{
public:
    int _status;    //响应状态码
    std::unordered_map<std::string, std::string> _headers; //报头
    std::string _body; //响应报头
    bool _redirect_flag; //重定向标志
    std::string _redirect_url; //重定向url
public:
    HttpResponse()
    :_status(200), _redirect_flag(false)
    {}
    HttpResponse(int status)
    :_status(status), _redirect_flag(false)
    {}
    //报头的插入，查询，获取
    void SetHeader(const std::string &key, const std::string &val)
    {
        _headers.insert(std::make_pair(key, val));
    }
    bool HasHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if(it == _headers.end())
        {
            return false;
        }
        return true;
    }
    std::string GetHeader(const std::string &key) const
    {
        auto it = _headers.find(key);
        if(it == _headers.end())
        {
            return "";
        }
        return it->second;
    }
    //设置正文
    void SetContent(const std::string &body, const std::string &type = "text/html")
    {
        _body = body;
        SetHeader("Content-Type", type);
    }
    //设置重定向
    void SetRedirect(const std::string &url)
    {
        _redirect_flag = true;
        _redirect_url = url;
    }
    //判断是否是短连接
    bool Close() const
    {
        if(HasHeader("Connection") && GetHeader("Connection") == "keep-alive")
        {
            
            return false;
        }
        return true;
    }  
};

enum HttpRecvStatus
{
    RECV_HTTP_ERR,
    RECV_HTTP_LINE,
    RECV_HTTP_HEAD,
    RECV_HTTP_BODY,
    RECV_HTTP_OVER
};
#define MAX_LINE 8192 //8k
class HttpContext
{
private:
    int _resp_status;               //响应状态码
    HttpRecvStatus _recv_status;    //当前接收及解析的阶段状态
    HttpRequest _request;           //已经解析得到的请求信息
private:
    bool RecvHttpLine(Buffer *buffer)
    {
        if(_recv_status != RECV_HTTP_LINE) return false;
        std::string line = buffer->GetOneLineAndPop();
        //如果没有一行数据
        if(line == "")
        {
            //缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长了都不足一行，这是有问题的
            if(buffer->ReadAbleSize() > 8192)
            {
                _resp_status = 413;//URI TOO LONG
                _recv_status = RECV_HTTP_ERR;
                return false;
            }
            //缓冲区中数据不足一行，但是也不多，就等等新数据的到来
            else
            {
                return true;
            }
        }
        //如果一行数据太大，无法解析
        if(line.size() > MAX_LINE)
        {
            _resp_status = 413;//URI TOO LONG
            _recv_status = RECV_HTTP_ERR;
            return false;
        }

        bool ret = ParseHttpLine(line);
        if(ret == false) return false;
        _recv_status = RECV_HTTP_HEAD;
        return true;
    }
    bool ParseHttpLine(const std::string &line)
    {
        std::smatch matchs;

        //HTTP请求行格式：  GET /index.html?username=pwd&passwd=123456 HTTP/1.1\r\n
        //避免一些不规范的请求，把GET -> get 所以我没要进行大小写忽略
        std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\r|\r\n)", std::regex::icase);
        bool ret = std::regex_match(line, matchs, e);
        if(ret == false)
        {
            _resp_status = 400; //BAD REQUEST
            _recv_status = RECV_HTTP_ERR;
            return false;
        }
        //0 GET /index.html?username=pwd&passwd=123456 HTTP/1.1
        //1 GET
        //2 /index.html
        //3 username=pwd&passwd=123456
        //4 HTTP/1.1
        _request._method = matchs[1];
        //转成大写
        std::transform(_request._method.begin(), _request._method.end(), _request._method.begin(), ::toupper);
        _request._path = Util::UrlDecode(matchs[2], false);
        //如果请求中有查询字符串
        if(matchs.size() == 5)
        {
            std::string query_str = Util::UrlDecode(matchs[3], true);
            std::vector<std::string> query_str_arry;
            Util::Split(query_str, "&", &query_str_arry);
            for(auto &str : query_str_arry)
            {
                auto pos = str.find("=");
                if(pos == std::string::npos)
                {
                    _resp_status = 400; //BAD REQUEST
                    _recv_status = RECV_HTTP_ERR;
                    return false;
                }
                std::string key = str.substr(0, pos);
                std::string value = str.substr(pos + 1);
                _request.SetParam(key, value);
            }
            _request._version = matchs[4];
        }
        //如果请求中没有查询字符串
        else if(matchs.size() == 4)
        {
            _request._version = matchs[3];
        }
        else
        {
            //请求不规范
            _resp_status = 400; //BAD REQUEST
            _recv_status = RECV_HTTP_ERR;
            return false;
        }
        
        return true;
    }

    bool RecvHttpHead(Buffer *buffer)
    {
        if(_recv_status != RECV_HTTP_HEAD) return false;
        while(1)
        {
            //1. 获取一行数据，带有末尾的换行
            std::string line = buffer->GetOneLineAndPop();
            //2. 需要考虑的一些要素：缓冲区中的数据不足一行， 获取的一行数据超大
            if(line == "")
            {
                //缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长了都不足一行，这是有问题的
                if(buffer->ReadAbleSize() > MAX_LINE)
                {
                    _resp_status = 413;//URI TOO LONG
                    _recv_status = RECV_HTTP_ERR;
                    return false;
                }
                //缓冲区中数据不足一行，但是也不多，就等等新数据的到来
                else
                {
                    return true;
                }
            }
            if(line.size() > MAX_LINE)
            {
                _resp_status = 413;//URI TOO LONG
                _recv_status = RECV_HTTP_ERR;
                return false;
            }

            if (line == "\n" || line == "\r\n") {
                break;
            }
            bool ret = ParseHttpHead(line);
            if(ret == false) return false;
        }
        _recv_status = RECV_HTTP_BODY;
        return true;
    }
    bool ParseHttpHead(std::string &line)
    {
        //key: val\r\n
        if(line.back() == '\n') line.pop_back();
        if(line.back() == '\r') line.pop_back();
        auto pos = line.find(':');
        if(pos == std::string::npos)
        {
            _resp_status = 400; //BAD REQUEST
            _recv_status = RECV_HTTP_ERR;
            return false;
        }
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 2);
        _request.SetHeader(key, value);
        return true;

    }
    bool RecvHttpBody(Buffer *buffer)
    {
        if(_recv_status != RECV_HTTP_BODY) return false;
        //正文的长度
        size_t body_size = _request.BodySize();
        if(body_size == 0) 
        {
            _recv_status = RECV_HTTP_OVER;
            return true;
        }
        // 当前已经接收了多少正文,其实就是往 _request._body 中放了多少数据了
        size_t real_size = body_size - _request._body.size();
        //缓冲区中数据，无法满足当前正文的需要，数据不足，取出数据，然后等待新数据到来
        if(buffer->ReadAbleSize() < real_size)
        {
            _request._body.append(buffer->ReadPosition(), buffer->ReadAbleSize());
            buffer->MoveReadBack(buffer->ReadAbleSize());
            return true;
        }
        _request._body.append(buffer->ReadPosition(), real_size);
        buffer->MoveReadBack(real_size);
        _recv_status = RECV_HTTP_OVER;
        return true;
    }

public:
    HttpContext()
    :_resp_status(200), _recv_status(RECV_HTTP_LINE)
    {}
    void Reset()
    {
        _resp_status = 200;
        _recv_status = RECV_HTTP_LINE;
        _request.Reset();
    }
    int RespStatus() 
    {
        return _resp_status;
    }
    HttpRecvStatus RecvStatus() 
    {
        return _recv_status;
    }
    HttpRequest& Request() 
    {
        return _request;
    }

    //接受并解析http请求
    void RecvHttpRequest(Buffer *buffer)
    {
          //不同的状态，做不同的事情，但是这里不要break， 因为处理完请求行后，应该立即处理头部，而不是退出等新数据
        switch(_recv_status)
        {
            case RECV_HTTP_LINE: RecvHttpLine(buffer);
            case RECV_HTTP_HEAD: RecvHttpHead(buffer);
            case RECV_HTTP_BODY: RecvHttpBody(buffer);
        }
    }
};

class HttpServer
{
private:
    using Handler = std::function<void(const HttpRequest&, HttpResponse*)>;
    // using Handlers = std::unordered_map<std::regex, Handler>;
    using Handlers = std::vector<std::pair<std::regex, Handler>>;

    //关于请求方法的处理路由表
    Handlers _get_route;
    Handlers _post_route;
    Handlers _put_route;
    Handlers _delete_route;
    std::string _base_dir; //静态资源相对根目录
    TcpServer _server; //tcp服务器负责io操作
private:
    void OnConnect(const PtrConnection& conn)
    {
        conn->SetContext(HttpContext());
        INFO_LOG("a new connection: %p", conn.get());
    }
    void OnClose(const PtrConnection& conn)
    {
        INFO_LOG("close a connection: %p", conn.get());
    }
    void ErrorHandler(const HttpRequest& req, HttpResponse* resp)
    {
        //1. 组织一个错误展示页面
        std::string body;
        body += "<html>";
        body += "<head>";
        body += "<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
        body += "</head>";
        body += "<body>";
        body += "<h1>";
        body += std::to_string(resp->_status);
        body += " ";
        body += Util::StatusDesc(resp->_status);
        body += "</h1>";
        body += "</body>";
        body += "</html>";
        //2. 将页面数据，当作响应正文，放入rsp中
        resp->SetContent(body, "text/html");
    }
    //将HttpResponse中的要素按照http协议格式进行组织，发送
    void WriteReponse(const PtrConnection& conn, const HttpRequest& req, HttpResponse& resp)
    {
        //1. 先完善头部字段
        if(req.Close() == true)
        {
            resp.SetHeader("Connection", "close");
        }
        else
        {
            resp.SetHeader("Connection", "keep-alive");
        }

        if(resp._body.empty() == false && resp.HasHeader("Content-Length") == false)
        {
            resp.SetHeader("Content-Length", std::to_string(resp._body.size()));
        }

        if (resp._body.empty() == false && resp.HasHeader("Content-Type") == false) 
        {
            resp.SetHeader("Content-Type", "application/octet-stream");
        }

        if(resp._redirect_flag == true)
        {
            resp.SetHeader("Location", resp._redirect_url);
        }

        std::stringstream ssm;
        ssm << req._version << " " << resp._status << " " << Util::StatusDesc(resp._status) << "\r\n";
        for(auto & head : resp._headers)
        {
            ssm << head.first << ": " << head.second << "\r\n";
        }
        ssm << "\r\n";
        ssm << resp._body;
        conn->Send(ssm.str().c_str(), ssm.str().size());
    }
    //判断是否是静态资源获取
    bool IsFileHandler(HttpRequest& req)
    {
        //必须设置了根目录
        if(_base_dir.empty())
        {
            return false;
        }
        // 2. 请求方法，必须是GET / HEAD请求方法
        if(req._method != "GET" && req._method != "HEAD")
        {
            return false;
        }
        // 3. 请求的资源路径必须是一个合法路径
        if(Util::IsValidPath(req._path) == false)
        {
            return false;
        }
        // 4. 请求的资源必须存在,且是一个普通文件
        //    有一种请求比较特殊 -- 目录：/, /image/， 这种情况给后边默认追加一个 index.html
        // index.html    /image/a.png
        // 不要忘了前缀的相对根目录,也就是将请求路径转换为实际存在的路径  /image/a.png  ->   ./wwwroot/image/a.png
        std::string path_str = _base_dir + req._path;
        if(req._path.back() == '/')
        {
            path_str += "index.html";
        }
        if(Util::IsRegular(path_str) == false)
        {
            return false;
        }
        //走到这里可以确定是静态资源获取了，可以修改path了
        req._path = path_str;
        return true;
    }
    //静态资源的请求处理 --- 将静态资源文件的数据读取出来，放到rsp的_body中, 并设置mime
    void FileHandler(HttpRequest& req, HttpResponse* resp)
    {
        std::string body_str;
        bool ret = Util::ReadFile(req._path, &resp->_body);
        if(ret == false) return;
        std::string mime = Util::ExtMime(req._path);
        resp->SetHeader("Content-Type", mime);
        return;

    }
    void Dispatcher(HttpRequest& req, HttpResponse* resp, Handlers& handlers)
    {
        for(auto & e : handlers)
        {
            const std::regex &pattern = e.first;
            const Handler &handler = e.second;
            bool ret = std::regex_match(req._path, req._smatch, pattern);
            if(ret == true)
            {
                return handler(req, resp);
            }
        }
        resp->_status = 404;

    }
    void Route(HttpRequest& req, HttpResponse* resp)
    {
        if(IsFileHandler(req))
        {
            return FileHandler(req, resp);
        }
        if(req._method == "GET")
        {
            return Dispatcher(req, resp, _get_route);
        }
        else if(req._method == "PUT")
        {
            return Dispatcher(req, resp, _put_route);
        }
        else if(req._method == "POST")
        {
            return Dispatcher(req, resp, _post_route);
        }
        else if(req._method == "DELETE")
        {
            return Dispatcher(req, resp, _delete_route);
        }
        resp->_status = 405; //Method Not Allowed
    }


    void OnMessage(const PtrConnection& conn, Buffer *buffer)
    {
        while(buffer->ReadAbleSize() > 0)
        {
            //1.获取上下文
            HttpContext* context = conn->GetContext()->Get<HttpContext>();
            //2.对请求进行解析
            context->RecvHttpRequest(buffer);
            HttpRequest& req = context->Request();
            HttpResponse resp(context->RespStatus());
            //解析失败,进行错误响应，关闭连接
            if(context->RespStatus() >= 400)
            {
                ErrorHandler(req, &resp);
                WriteReponse(conn, req, resp);
                //出错了就把缓冲区清空!!!!非常重要，因为Shutdown中如果缓冲区中有数据还有进行处理，这样的话造死递归
                buffer->Clear();
                context->Reset();
                buffer->MoveReadBack(buffer->ReadAbleSize());

                conn->Shutdown();
                return;
            }
            //如果当前没有收到的请求不完整，等新数据到来再重新继续处理
            if(context->RecvStatus() != RECV_HTTP_OVER)
            {
                return;
            }

            //3. 请求路由 + 业务处理
            Route(req, &resp);
            
            //4.组织响应进行回复
            WriteReponse(conn, req, resp);
            //5.重置上下文
            context->Reset();
            //6.如果是短连接，关闭连接
            if(resp.Close() == true)
            {
                conn->Shutdown();
            }
        }

    }
public:
    HttpServer(int port, int timeout = DEFAULT_TIMEOUT)
    :_server(port)
    {
        _server.SetConnectCallback(std::bind(&HttpServer::OnConnect, this, std::placeholders::_1));
        _server.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
        _server.SetCloseCallback(std::bind(&HttpServer::OnClose, this, std::placeholders::_1));
        _server.EnableInactiveDestroy(timeout);
    }
    void Get(const std::string& pattern, const Handler& handler)
    {
        _get_route.push_back(std::make_pair(std::regex(pattern), handler));
    }
    void Put(const std::string& pattern, const Handler& handler)
    {
        _put_route.push_back(std::make_pair(std::regex(pattern), handler));

    }
    void Post(const std::string& pattern, const Handler& handler)
    {
        _post_route.push_back(std::make_pair(std::regex(pattern), handler));

    }
    void Delete(const std::string& pattern, const Handler& handler)
    {
        _delete_route.push_back(std::make_pair(std::regex(pattern), handler));

    }
    void SetBaseDir(const std::string& path)
    {
        _base_dir = path;
    }
    void SetThreadCount(int count)
    {
        _server.SetThreadCount(count);
    }
    void Listen()
    {
        _server.Start();
    }
};