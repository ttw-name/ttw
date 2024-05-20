#ifndef __TTW_SOCKET_H
#define __TTW_SOCKET_H

#include <memory>
#include <string>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <mutex>
#include <condition_variable>

#include "address.h"
#include "ioscheduler.h"
#include "buffer.h"

#define TCP SOCK_STREAM
#define UDP SOCK_DGRAM

#define IPv4 AF_INET

namespace ttw {
 
    class Socket : public std::enable_shared_from_this<Socket> {
    public:
        std::mutex m_mutex;

        using ptr = std::shared_ptr<Socket>;
        static Socket::ptr CreateTCPSoket(IOScheduler* scheduler);
        static Socket::ptr CreateTCPSoket(IPv4Address::ptr address,IOScheduler* scheduler= nullptr);

        Socket(int family = IPv4, int type = TCP, int protocol = 0, IOScheduler* scheduler = nullptr);
        virtual ~Socket();

        uint64_t getTimeout();
        void setTimeout(uint64_t time);
        bool getOption(int level, int option, void* result, socklen_t* len);
        template<class T>
        bool getOption(int level, int option, T& result){
            socklen_t len = sizeof(T);
            return getOption(level, option, &result, &len);
        }

        bool setOption(int level, int option, const  void* result, socklen_t len);

        void setRecvTimeout(int64_t v);

        template<class T>
        bool setOption(int level, int option, const T& val){
            return setOption(level, option, &val, sizeof(T));
        }

        Socket::ptr accept();

        bool bind(const IPv4Address::ptr addr);

        int connect(const IPv4Address::ptr addr, uint64_t timeout = -1);

        bool reconnect(uint64_t timeout = -1);

        bool listen(int backlog = SOMAXCONN);

        bool close();

        int send(void* buffer, size_t length, int flags = 0);

        int send(iovec* buffers, size_t length, int flags = 0);

        int sendTo(iovec* buffers, size_t length, IPv4Address::ptr to, int flags = 0);

        int sendTo(void* buffers, size_t length, IPv4Address::ptr to, int flags = 0);

        int recv(void* buffer, size_t length, int flags = 0);

        int recv(iovec* buffers, size_t length, int flags = 0);

        int recvFrom(void *buffer, size_t length, IPv4Address::ptr from, int flags = 0);

        int recvFrom(iovec* buffers, size_t length, IPv4Address::ptr from, int flags = 0);

        IPv4Address::ptr getRemoteAddress();

        IPv4Address::ptr getLocalAddress();


        int getFamiy();

        int getType();

        bool isConned();

        bool isValid() const;

        int getError();

        IOScheduler* getScheduler() const { return m_scheduler; }




        std::string toString() const;


        bool cancelRead();

        bool cancelWrite();

        bool cancelAccept();

        bool cancelAll();

        void deleteEventCb(); 
        
    private:
        void initSocket();
        void newSocket();
        bool init(int sock);

        
    private:

        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConned;
        uint64_t m_timeOut;
        uint64_t m_receipttime;
        uint64_t m_sendtime;
        
        

        IPv4Address::ptr m_local;
        IPv4Address::ptr m_remote;
        IOScheduler* m_scheduler ;

    };

}


#endif