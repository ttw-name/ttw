#include "http/http_parser.h"
#include "log.h"


ttw::Logger::ptr log_(new ttw::Logger("parser"));

const char test_request_data[] =    "GET /index.thml HTTP/1.1\r\n"
                                    "Host: 192.168.80.128:8080\r\n"
                                    "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:123.0) Gecko/20100101 Firefox/123.0\r\n"
                                    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
                                    "Accept-Language: en-US,en;q=0.5\r\n"
                                    "Accept-Encoding: gzip, deflate\r\n"
                                    "Connection: keep-alive\r\n"
                                    "Upgrade-Insecure-Requests: 1\r\n\r\n"
                                    "123456789";



void test_request() {
    ttw::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    LOG_ERROR(log_) << "execute rt=" << s
        << "has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength();
    tmp.resize(tmp.size() - s);
    LOG_INFO(log_) << std::endl << parser.getData()->toString();
    LOG_INFO(log_) << tmp;
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
        "ETag: \"51-47cf7e6ee8400\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 81\r\n"
        "Cache-Control: max-age=86400\r\n"
        "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
        "Connection: Close\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>\r\n"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
        "</html>\r\n";

void test_response() {
    ttw::http::HttpResponseParser parser;
     std::string tmp = test_response_data;
     size_t s = parser.execute(&tmp[0], tmp.size(), true);
     LOG_ERROR(log_) << "execute rt=" << s
        << " has_error=" << parser.hasError()
         << " is_finished=" << parser.isFinished()
         << " total=" << tmp.size()
         << " content_length=" << parser.getContentLength()
        << " tmp[s]=" << tmp[s];

    tmp.resize(tmp.size() - s);

    LOG_INFO(log_) << std::endl << parser.getData()->toString();
    LOG_INFO(log_) << tmp;
}

int main(int argc, char** argv) {
    new ttw::http::HttpRequest();
    new ttw::http::HttpResponse();

    test_request();
    LOG_INFO(log_) << "--------------";
    test_response();
    return 0;
}
