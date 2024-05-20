#ifndef __TTW__TCPSERVER_H
#define __TTW__TCPSERVER_H

#include <memory>

#include "socket.h"
#include "ioscheduler.h"
#include "address.h"


namespace ttw {
    class TcpServer : public std::enable_shared_from_this<TcpServer>{
    public:
        using ptr = std::shared_ptr<TcpServer>;
        TcpServer(int works, const std::string& name, uint64_t timeout = (uint64_t)4000);
        virtual ~TcpServer();
        virtual bool bind(IPv4Address::ptr addr);
        virtual bool bind(const std::string &addr, int port = 80);

        virtual bool start();
        virtual void stop();

        std::string getName() const { return m_name; }
        void setName(const std::string& name){ m_name = name; }
        
        bool isStop() const { return m_stop; }

        Socket::ptr getSocks() const { return m_accpetSocket; }
        IOScheduler::ptr getScheduler(){
            return m_scheduler;
        }

        void join();

    protected:
        virtual void handClient(Socket::ptr sock);
        void startAcceptr();

    private:
        //std::vector<Socket::ptr> m_socks;
        Socket::ptr m_accpetSocket;
        IOScheduler::ptr m_scheduler;
        std::string m_name;
        uint64_t m_timeout;
        bool m_stop;

    };
}



#endif