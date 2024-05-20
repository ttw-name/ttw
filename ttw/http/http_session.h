

#ifndef __TTW__HTTP_SERSION_H
#define __TTW__HTTP_SERSION_H

#include "ttw/socket.h"
#include "ttw/log.h"
#include "http.h"
#include "http_parser.h"

namespace ttw {
    namespace http{

        class HttpSession {
        public:

            using ptr  = std::shared_ptr<HttpSession>;
            HttpSession(ttw::Socket::ptr sock);
            std::shared_ptr<HttpRequest> recvRequest();
            int sendResponse(std::shared_ptr<HttpResponse> rsp);
            int recvlen(void* buffer, size_t len);
            int sendlen(void* buffer, size_t len);
            void close();
        private:
            Socket::ptr m_sock;
        };


    }
}


#endif