
#include "tcpserver.h"
#include "log.h"
#include "config.cc"
#include <thread>
#include <chrono>


ttw::Logger::ptr log_(new ttw::Logger("echo"));


class EServer : public ttw::TcpServer{
    public:
        EServer(int work);
        void handClient(ttw::Socket::ptr sock) override;
    
};

EServer::EServer(int work):TcpServer(work, "ttw1.0", 5000) {
    
}

void EServer::handClient(ttw::Socket::ptr sock){
    LOG_DEBUG(log_) << "handClicent ";

    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //LOG_DEBUG(logger_) << "handCliinet Curoutine id = " << ttw::Coroutine::GetCoroutineId();
   
        
        void* date = malloc(1024);
        LOG_DEBUG(logger_) << "to recv Cor usecount = " << ttw::Coroutine::GetThis().use_count();
        int n = sock->recv((void*)date, 1024);
        LOG_DEBUG(logger_) << "afet recv Cor usecount = " << ttw::Coroutine::GetThis().use_count();
        if(n > 0){
            LOG_DEBUG(log_) << std::endl <<std::string((char*)date);
        }
        //LOG_DEBUG(log_) << n;
        
        if(n > 0 )
            sock->send(date, n);

        //free(date);
        LOG_DEBUG(log_) << "hand end: ";

}

void run(){
    //logger_->setLevel(ttw::LogLevel::INFO);
    EServer::ptr e(new EServer(1));
    if(!e->bind("192.168.80.128", 8080)){
        LOG_ERROR(log_) << "bind fail!";
    }
    LOG_DEBUG(logger_) << "sssssssssssss";
    e->start();
    LOG_DEBUG(logger_) << "sssssssssssss";
    int a ;
    std::cin >> a;
}



int main(){
    run();
    
    return 0;
}