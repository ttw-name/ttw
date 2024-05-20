#pragma once

#include <list>

#include "ttw/tcpserver.h"
#include "servlet.h"
#include "http_session.h"



namespace ttw {
namespace http{
    class HttpServer : public TcpServer {
public:

    using ptr =  std::shared_ptr<HttpServer>;


    HttpServer(int threads, const std::string& name, bool keepalive = false);

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}

    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

    void addAfterCb(std::function<void()> val, int index = 0);
    void addBeforeCb(std::function<void()> val, int index = 0);

    void delAfterCb(int index = 0);

    void delBeforeCb(int index = 0);
protected:
     void handClient(Socket::ptr sock) override;
private:

    bool m_keepLive;

    ServletDispatch::ptr m_dispatch;
    std::list<std::function<void()>> m_beforeConnCbs;
    std::list<std::function<void()>> m_afterConnCbs;
    std::shared_mutex m_mutex;
};
}
}