

#include "tcpserver.h"
#include "log.h"

std::shared_ptr<ttw::Logger> log_(new ttw::Logger("tepserver"));

namespace ttw {

    TcpServer::TcpServer(int works, const std::string& name, uint64_t timeout){
        m_scheduler.reset(new IOScheduler(name, false, works));
        m_name = name;
        m_stop = true;
        m_timeout = timeout;
        m_scheduler->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_DEBUG(log_) << "TcpServer()";
    }

    TcpServer::~TcpServer(){
        LOG_DEBUG(log_) << "~TcpServer()";
        m_scheduler.reset();
        if(m_accpetSocket)
            m_accpetSocket->close();
        LOG_DEBUG(log_) << "~TcpServer()";
    }

    bool TcpServer::bind(IPv4Address::ptr addr){
        Socket::ptr sock = Socket::CreateTCPSoket(addr, m_scheduler.get());
        if(!sock->bind(addr)){
            return false;
        }

        if(!sock->listen()){
            return false;
        }

        m_accpetSocket = sock;
        return true;
    }

    bool TcpServer::bind(const std::string& addr, int port){
        std::shared_ptr<IPv4Address> addp(new IPv4Address(addr, port));
        LOG_DEBUG(log_) << addp->toString();
        return  bind(addp);
    }


   void TcpServer::handClient(Socket::ptr sock){

   }
    
    void TcpServer::stop(){
        m_stop = true;
        m_accpetSocket->close();
    }

    bool TcpServer::start(){
        
        if(!m_stop){
            return true;
        }
        m_stop = false;
        m_scheduler->run(std::bind(&ttw::TcpServer::startAcceptr, shared_from_this()));
        return true;

    }

    void TcpServer::startAcceptr(){
        while(!m_stop){

            Socket::ptr sock = m_accpetSocket->accept();

            if(sock == nullptr){
                continue;
            }
            if(!sock->isValid()){
                continue;
            }
            
            sock->setRecvTimeout(5000);
            sock->setTimeout(m_timeout);
            m_scheduler->run(std::bind(&TcpServer::handClient, shared_from_this(), sock));
        }
    }

    void TcpServer::join(){
        m_scheduler->join();
    }

}