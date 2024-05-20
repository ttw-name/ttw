

#ifndef __TT__HTTP_H
#define __TT__HTTP_H

#include <memory>
#include <list>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h>






namespace ttw {
namespace http{

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \


  /* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \


enum class HttpMethod {
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

enum class HttpStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod(const std::string& m);

HttpMethod CharsToHttpMethod(const char* m);

const char* HttpMethodToString(const HttpMethod& m);

const char* HttpStatusToString(const HttpStatus& s);



    // class HttpServer : public TcpServer{
    // public:
    //     using ptr = std::shared_ptr<HttpServer>;

    // private:
    //     std::string m_name;

        // std::list<std::function<void()>> m_beforeConnCb;
        // std::list<std::function<void()>> m_afterConnCb;
    // };


    class HttpMsg{
    public:

        struct caseInsensitiveLess{
            bool operator()(const std::string& a, const std::string& b) const ;
        };

        using MapType = std::map<std::string, std::string, caseInsensitiveLess>;
        
        HttpMsg(uint8_t version = 0x11, bool close = true);


        uint8_t getVersion() const { return m_version; }
        void setVersion(uint8_t v) { m_version = v; }
        std::string& getBody() { return m_body; }
        bool isClose() { return m_close; }
        void setBody(const std::string& body) { m_body = body; }
        MapType& getHeaders() { return m_headers; }
        void setHeaders(MapType& val) { m_headers = val; }

        MapType& getCookies() { return m_cookies; }
        void setCookies(MapType& val) { m_cookies = val; }

        void setHeaderValue(const std::string& key, const std::string& value);

        std::string getHeaderValue(const std::string& key);

        void setCookieValue(const std::string& key, const std::string& value);

        std::string getCookieValue(const std::string& key);
        void addHeaderKey(const std::string& key, const std::string& value);
        void delHeaderKey(const std::string& key);
        void addCookieKey(const std::string& key, const std::string& value);
        void delCookieKey(const std::string& key);

    protected:
       
        std::string getValue(const MapType& map, const std::string& key);
        void setValue(MapType& map, const std::string& key, const std::string value);

        void delKey(MapType& map, const std::string& key);
        void addKey(MapType& map, const std::string& key, const std::string& value);

        //bool mapMatch(const std::string& a, const std::string& b);
        
        
    protected:
        uint8_t m_version;
        bool m_close;
        std::string m_body;
        MapType m_headers;
        MapType m_cookies;
    };

    class HttpRequest : public HttpMsg{ 
    public:
        using ptr = std::shared_ptr<HttpRequest>;
        HttpRequest(uint8_t version = 0x11, bool close = false);
        void setMethod(HttpMethod method) { m_method = method; }
        HttpMethod getMethod() const { return m_method; }
        void setPath(const std::string& path) { m_path = path; }
        void setQuery(const std::string& query);
        std::string getQuery(const std::string& key, std::string def = "");
        const std::string& getPath() const { return m_path; }
        std::string toString();
        void init();
    private:
        HttpMethod m_method;
        std::string m_path;
        MapType m_query;
    };

    class HttpResponse: public HttpMsg{
    public:
        HttpResponse(uint8_t version = 0x11, bool close = true);
        using ptr = std::shared_ptr<HttpResponse>;
        void setStatus(HttpStatus status) { m_status = status; }
        void setReason(const std::string& reason) { m_reason = reason; }
        std::string toString();
        void setCookie(const std::string& key, const std::string& val, time_t expired, const std::string& path, const std::string& domain, bool secure);
        void redirect(const std::string& addr);
    private:
        HttpStatus m_status;
        std::string m_reason;
        std::vector<std::string> m_cookiesv;
    };
};
};


#endif