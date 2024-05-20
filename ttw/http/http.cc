

#include "http.h"
#include "util.h"

namespace ttw {

namespace http{



HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, string) \
    if(strcmp(#string, m.c_str()) == 0) { \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string) \
    if(strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

const char* HttpMethodToString(const HttpMethod& m) {
    uint32_t idx = (uint32_t)m;
    if(idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
        return "<unknown>";
    }
    return s_method_string[idx];
}

const char* HttpStatusToString(const HttpStatus& s) {
    switch(s) {
#define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "<unknown>";
    }
}


        HttpMsg::HttpMsg(uint8_t version, bool close): m_version(version), m_close(close) {}
        void HttpMsg::setHeaderValue(const std::string& key, const std::string& value){
            setValue(m_headers, key, value);
        }

        std::string HttpMsg::getHeaderValue(const std::string& key){
           return getValue(m_headers, key);
        }

        void HttpMsg::setCookieValue(const std::string& key, const std::string& value){
            setValue(m_cookies, key, value);
        }

        std::string HttpMsg::getCookieValue(const std::string& key){
            return getValue(m_cookies, key);
        }

        std::string HttpMsg::getValue(const MapType& map, const std::string& key){
            auto it = map.find(key);
            if(it != map.end()){
                return it->second;
            }
            return "0";
        }

        void HttpMsg::setValue(MapType&  map, const std::string& key, const std::string value){
            auto it = map.find(key);
            if(it != map.end()){
                it->second = value;
            }
        }

        void HttpMsg::addHeaderKey(const std::string& key, const std::string& value){
            addKey(m_headers, key, value);
        }

        void HttpMsg::delHeaderKey(const std::string& key){
            delKey(m_headers, key);

        }

        void HttpMsg::addCookieKey(const std::string& key, const std::string& value){
            m_cookies[key] = value;
        }

        void HttpMsg::delCookieKey(const std::string& key){
            delKey(m_cookies, key);
        }

        void HttpMsg::delKey(MapType&  map, const std::string& key){
            
            auto it = map.find(key);
            if(it != map.end()){
                map.erase(it);
            }

        }

        void HttpMsg::addKey(MapType&  map, const std::string& key, const std::string& value){
            map[key] = value;
        }

        bool HttpMsg::caseInsensitiveLess::operator()(const std::string& a, const std::string& b) const {
            std::string aLower = a;
            std::string bLower = b;
            std::transform(aLower.begin(), aLower.end(), aLower.begin(), [](unsigned char c){ return std::tolower(c); });
            std::transform(bLower.begin(), bLower.end(), bLower.begin(), [](unsigned char c){ return std::tolower(c); });
            return aLower < bLower;
        }

        void HttpRequest::setQuery(const std::string& query){
            std::stringstream ss(query);
            std::string str;
            while(std::getline(ss, str, '&')){
                size_t pos = str.find('=');
                if(pos != std::string::npos){
                    std::string key = str.substr(0, pos);
                    std::string value = str.substr(pos + 1);
                    m_query[key] = value;
                }
            }
        }

        std::string HttpRequest::getQuery(const std::string& key, std::string def){
            auto it = m_query.find(key);
            if(it == m_query.end()){
                return def;
            }else{
                return m_query[key];
            }
        }

        std::string HttpRequest::toString(){
            bool nofirst = false;
            std::stringstream ss;
            ss  << HttpMethodToString(m_method) << " "
                << m_path;

            if(m_query.empty()){
                ss << "";
            }else{
                ss << "?";
                for(auto it = m_query.begin(); it != m_query.end(); ++it){
                    if(nofirst){
                        ss << "&";
                    }
                    ss << it->first << "=" << it->second;
                    nofirst = true;
                }
            }

            ss  << " HTTP/"
                << ((uint32_t)(m_version >> 4))
                << "."
                << ((uint32_t)(m_version & 0x0F))
                << "\r\n";

            for(auto& i : m_headers) {
                if(strcasecmp(i.first.c_str(), "connection") == 0) {
                    continue;
                }
                ss << i.first << ": " << i.second << "\r\n";
            }

            if(!m_body.empty()) {
                ss << "content-length: " << m_body.size() << "\r\n\r\n"
                   << m_body;
            } else {
                ss << "\r\n";
            }  
            return ss.str();
        }
        std::string HttpResponse::toString(){
            std::stringstream os;
            os  << "HTTP/"
                << ((uint32_t)(m_version >> 4))
                << "."
                << ((uint32_t)(m_version & 0x0F))
                << " "
                << (uint32_t)m_status
                << " "
                << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
                << "\r\n";

            for(auto& i : m_headers) {
                if(strcasecmp(i.first.c_str(), "connection") == 0) {
                    continue;
                }   
                os << i.first << ": " << i.second << "\r\n";
            }
            for(auto& i : m_cookiesv) {
                os << "Set-Cookie: " << i << "\r\n";
            }
            os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
            if(!m_body.empty()) {
                os << "content-length: " << m_body.size() << "\r\n\r\n"
                << m_body;
            } else {
                os << "\r\n";
            }
            return os.str();
        }

        void HttpRequest::init(){
            std::string conn = getHeaderValue("connection");
            if(!conn.empty()) {
                if(strcasecmp(conn.c_str(), "keep-alive") == 0) {
                    m_close = false;
                } else {
                    m_close = true;
                }
            }
        }

        void HttpResponse::setCookie(const std::string& key, const std::string& val,
                             time_t expired, const std::string& path,
                             const std::string& domain, bool secure) {
            std::stringstream ss;
            if(key.empty()){
                ss << val;
            }else{
                ss << key << "=" << val;
            }
            
            if(expired > 0) {
                ss << ";expires=" << ttw::Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
            }
            if(!domain.empty()) {
                ss << ";domain=" << domain;
            }
            if(!path.empty()) {
                ss << ";path=" << path;
            }
            if(secure) {
                ss << ";secure";
            }
            m_cookiesv.push_back(ss.str());
        }

        void HttpResponse::redirect(const std::string& addr){
            m_status = HttpStatus::FOUND;
            addHeaderKey("Location", addr);
        }
        

        

        HttpRequest::HttpRequest(uint8_t version, bool close):HttpMsg(version, close){
            m_method = HttpMethod::GET;
            m_path = "/";
        }
        HttpResponse::HttpResponse(uint8_t version, bool close):HttpMsg(version, close){
            m_status = HttpStatus::OK;
            addHeaderKey("Server", "ttw/1.0.0");
        }

        

    }


}