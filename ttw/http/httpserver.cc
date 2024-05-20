

#include "httpserver.h"

#include "config.h"
#include "log.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

namespace ttw{
namespace http{

    HttpServer::HttpServer(int threads, const std::string& name, bool keep_live):TcpServer(threads, name, (uint64_t)-1)
                            ,m_keepLive(keep_live)  {
                            m_dispatch.reset(new ServletDispatch);


    }


    void HttpServer::addAfterCb(std::function<void()> val, int index){
        std::unique_lock<std::shared_mutex> w_lock(m_mutex);
        auto it = m_afterConnCbs.begin();
        for(;it != m_afterConnCbs.end(); ++it){
        }
        if(it == m_afterConnCbs.end()){
            m_afterConnCbs.push_back(val);
        }else{
            m_afterConnCbs.insert(it, val);
        }

        
    }
    void HttpServer::addBeforeCb(std::function<void()> val, int index){
        std::unique_lock<std::shared_mutex> w_lock(m_mutex);
        auto it = m_afterConnCbs.begin();
        for(;it != m_beforeConnCbs.end(); ++it){
        }
        if(it == m_beforeConnCbs.end()){
            m_beforeConnCbs.push_back(val);
        }else{
            m_beforeConnCbs.insert(it, val);
        }
    }
    void HttpServer::delAfterCb(int index){
        std::unique_lock<std::shared_mutex> w_lock(m_mutex);
        auto it = m_afterConnCbs.begin();
        for(;it != m_afterConnCbs.end(); ++it){
        }
        if(it == m_afterConnCbs.end()){
            m_afterConnCbs.pop_back();
        }else{
            m_afterConnCbs.erase(it);
        }
    }
    void HttpServer::delBeforeCb(int index){
        std::unique_lock<std::shared_mutex> w_lock(m_mutex);
        auto it = m_beforeConnCbs.begin();
        for(;it != m_beforeConnCbs.end(); ++it){
        }
        if(it == m_beforeConnCbs.end()){
            m_beforeConnCbs.pop_back();
        }else{
            m_beforeConnCbs.erase(it);
        }
    }


    void HttpServer::handClient(Socket::ptr sock){

        HttpSession::ptr session(new HttpSession(sock));
        do{
            auto req = session->recvRequest();
            if(!req){
                LOG_DEBUG(logger_) << "break";
                break;
            }
            
            HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_keepLive));
            
            
            m_dispatch->handle(req, rsp,session);
            session->sendResponse(rsp);
            if(true){
                break;
            }
                
        }while(1);

        session->close();
    }

}
}