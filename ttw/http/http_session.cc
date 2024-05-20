

#include "http_session.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

namespace ttw{
namespace http{


        HttpSession::HttpSession(std::shared_ptr<ttw::Socket> sock):m_sock(sock){
            
        }

        std::shared_ptr<HttpRequest> HttpSession::recvRequest() {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            std::shared_ptr<char> buffer(
                    new char[buff_size], [](char* ptr){
                        delete[] ptr;
                    });
            char* data = buffer.get();
            int offset = 0;
            do {
                int len = m_sock->recv(data + offset, buff_size - offset);
                if(len < 0) {
                    m_sock->close();
                    LOG_DEBUG(logger_) << "len : " << len << "erron : " << errno;
                    return nullptr;
                }
                len += offset;
                size_t nparse = parser->execute(data, len);
                if(parser->hasError()) {
                    m_sock->close();
                    LOG_DEBUG(logger_) << "hasError < 0";
                    return nullptr;
                }
                offset = len - nparse;
                if(offset == (int)buff_size) {
                    m_sock->close();
                    LOG_DEBUG(logger_) << "hasError > 0";
                    return nullptr;
                }
                if(parser->isFinished()) {
                    break;
                }
            } while(true);
            int64_t length = parser->getContentLength();
            LOG_DEBUG(logger_) << "length : " << length;
            if(length > 0) {

                //LOG_DEBUG(logger_) << "length : " << length;

                std::string body;
                body.resize(length);

                int len = 0;
                if(length >= offset) {
                    memcpy(&body[0], data, offset);
                    len = offset;
                } else {
                    memcpy(&body[0], data, length);
                    len = length;
                }
                length -= offset;
                if(length > 0) {
                    if(recvlen(&body[len], length) <= 0) {
                        m_sock->close();
                        return nullptr;
                    }
                }
                parser->getData()->setBody(body);
            }

            parser->getData()->init();
            return parser->getData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp) {
            std::stringstream ss;
            ss << rsp->toString();
            std::string data = ss.str();
            return sendlen((void*)data.c_str(), data.size());
        }

        int HttpSession::recvlen(void* buffer, size_t len){
            size_t offset = 0;
            int64_t left = len;
            while(left > 0) {
                int64_t len = m_sock->recv((char*)buffer + offset, left);
                if(len <= 0) {
                    return len;
                }
                offset += len;
                left -= len;
            }
            return len;
        }

        int HttpSession::sendlen(void* buffer, size_t len){
            size_t offset = 0;
            int64_t left = len;
            while(left > 0) {
                int64_t n = m_sock->send(((char* )buffer + offset), left);
                if(n <= 0) {
                    return n;
                }
                offset += n;
                left -= n;
            }
            return len;
        }

        void HttpSession::close(){
            if(m_sock)
                m_sock->close();
        }
}
}