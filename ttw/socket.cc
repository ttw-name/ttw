#include <semaphore.h>
#include <sys/time.h>


#include "socket.h"
#include "config.h"
#include "util.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

namespace ttw {
    Socket::ptr Socket::CreateTCPSoket(IOScheduler* scheduler){
        Socket::ptr sock(new Socket(IPv4, TCP, 0, scheduler));
        return sock;
    }
    Socket::ptr Socket::CreateTCPSoket(IPv4Address::ptr address, IOScheduler* scheduler){
        Socket::ptr sock(new Socket(address->getFamiy(), TCP, 0, scheduler));
        return sock;
    }

    Socket::Socket(int family, int type, int protocol, IOScheduler* scheduler)
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConned(false)
    ,m_timeOut(0)
    ,m_scheduler(scheduler)
    {

    }

    Socket::~Socket(){
        close();
    }

    uint64_t Socket::getTimeout(){
        return m_timeOut;
    }

    void Socket::setTimeout(uint64_t val){
        m_timeOut = val;
        m_receipttime = val;
        m_sendtime = val;
        
    }

    void Socket::setRecvTimeout(int64_t val){
        struct timeval tv{int(val / 1000) , int(val % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }

    bool Socket::getOption(int level, int option, void* result, socklen_t* len){
        int rt = getsockopt(m_sock, level, option, result, (socklen_t* )len);
        if(rt){
            LOG_ERROR(logger_) << "getoption erron! sock: " << m_sock;
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void* result, socklen_t len){
        int rt = setsockopt(m_sock, level, option, result, len);
        if(rt){
            LOG_ERROR(logger_) << "setoption erron! sock: " << m_sock;
            return false;
        }
        return true;
    }

    Socket::ptr Socket::accept(){
        ptr sock(new Socket(m_family, m_type, m_protocol, m_scheduler));
        int n = 0;
    while(true){
        n = ::accept(m_sock, nullptr, nullptr);
        if(n > 0){
            break;
        }
        while(n == -1 && errno == EINTR){
            n = ::accept(m_sock, nullptr, nullptr);
        }

        if(n == -1 && errno == EWOULDBLOCK){

            int ret = m_scheduler->addEvent(m_sock, Event::READ);
            if(ret != 0){
                LOG_DEBUG(logger_) << "addEvent fail";
                return nullptr;
            }else{
                Coroutine::ToSUSPENDED();
            }
        }
    }

        if(n > 0 ){
            if(!sock->init(n)){
                LOG_DEBUG(logger_) << "sock->init fail";
                return nullptr;
            }
            return sock;
        }
        LOG_DEBUG(logger_) << " erron = " << strerror(errno);
        return nullptr;
    }

    bool Socket::bind(const IPv4Address::ptr addr){
        if(!isValid()){
            newSocket();
        }
        int rt = ::bind(m_sock, addr->getAddress(), addr->getAddressLen());
        if(rt){
            LOG_ERROR(logger_) << "bind error errno = " << errno << " errstr= " << strerror(errno);
            return false;
        }
        getLocalAddress();
        return true;
    }

    bool Socket::reconnect(uint64_t timeout){
        if(!m_remote){
            LOG_ERROR(logger_) << " reconnect m_remotaddress is nullptr";
            return false;
        }
        m_local.reset();
        return connect(m_remote, timeout);
    }

    int Socket::connect(const IPv4Address::ptr addr, uint64_t timeout){
        m_remote = addr;
        if(!isValid()){
            newSocket();
            if(!isValid())
                return 0;
        }
        int n = 0;
        while(true){
            n = ::connect(m_sock, m_remote->getAddress(), m_remote->getAddressLen());

            while(n == -1 && errno == EINTR){
                n = ::connect(m_sock, m_remote->getAddress(), m_remote->getAddressLen());
            }

            if(n > 0){
                break;
            }

            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(timeout != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(timeout, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::READ);
                    }, weak_t, false);
                }

                int ret = scheduler->addEvent(m_sock, Event::READ);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }
                }
            }
        }
        m_isConned = true;
        getLocalAddress();
        getRemoteAddress();
        return n;
    }

    bool Socket::listen(int backlog){
        if(!isValid()){
            LOG_ERROR(logger_) << "listen error sock isvalid sock =  " << m_sock;
            return false;
        }
        if(::listen(m_sock, backlog)){
            LOG_ERROR(logger_) << "listen error errno = " << errno << " errstr = " << strerror(errno);
            return false;
        }
        setNonBlock(m_sock);
        return true;
    }
    bool Socket::close(){
        if(!m_isConned && m_sock == -1){
            return true;
        }

        m_isConned = false;
        //deleteEventCb();
        if(m_sock != -1){
            ::close(m_sock);
            m_sock = -1;
        }
        return true;
    }

    int Socket::send(void* buffer, size_t length, int flags){
        if(!isConned()){
            return -1;
            LOG_DEBUG(logger_) << "Close ?";
        }

        while(true){
            int n = ::send(m_sock, buffer, length, flags);
    
            while(n == -1 && errno == EINTR){
                n = ::send(m_sock, buffer, length, flags);
            }
            if(n > 0){
                return n;
            }
            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_sendtime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_sendtime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::WRITE);
                    }, weak_t, false);
                }
                int ret = scheduler->addEvent(m_sock, Event::WRITE);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }
                }
            }
        }
    }

    int Socket::send(iovec* buffers, size_t length, int flags){
        //output_->lock;
        if(!isConned()){
            return -1;
        }
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffers;
        msg.msg_iovlen = length;

        while(true){
            int n = ::sendmsg(m_sock, &msg, flags);

            while(n == -1 && errno == EINTR){
                n = ::sendmsg(m_sock, &msg, flags);
            }
            if(n > 0){
                return n;
            }

            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_sendtime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_sendtime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::WRITE);
                    }, weak_t, false);
                }
                int ret = scheduler->addEvent(m_sock, Event::WRITE);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }


                }
            }
        }
    }

    int Socket::sendTo(iovec* buffers, size_t length, IPv4Address::ptr to, int flags){
        if(!isConned()){
            return -1;
        }

        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffers;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddress();
        msg.msg_namelen = to->getAddressLen();

        while(true){
            int n = ::sendto(m_sock, &msg, length, flags, to->getAddress(), to->getAddressLen());

            while(n == -1 && errno == EINTR){
                n = ::sendto(m_sock, &msg, length, flags, to->getAddress(), to->getAddressLen());
            }
            if(n > 0){
                return n;
            }
            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_sendtime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_sendtime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::WRITE);
                    }, weak_t, false);
                }

                int ret = scheduler->addEvent(m_sock, Event::WRITE);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }
                }

            }
        }
    }


    int Socket::sendTo(void* buffer, size_t length, IPv4Address::ptr to, int flags){
        if(!isConned()){
            return -1;
        }


        while(true){
            int n = ::sendto(m_sock, buffer, length, flags, to->getAddress(), to->getAddressLen());

            while(n == -1 && errno == EINTR){
                n = ::sendto(m_sock, buffer, length, flags, to->getAddress(), to->getAddressLen());
            }

            if(n > 0){
                return n;
            }

            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_sendtime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_sendtime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::WRITE);
                    }, weak_t, false);
                }

                int ret = scheduler->addEvent(m_sock, Event::WRITE);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }

            
                }
            }

        }
    }

    int Socket::recv(void* buffer, size_t length, int flags){

        if(!isConned()){
            LOG_DEBUG(logger_) << "colse";
            return -1;
        }

        while(true){
            int n = ::recv(m_sock, buffer, length, flags);

            while(n == -1 && errno == EINTR){
                n = ::recv(m_sock, buffer, length, flags);
            }
            if(n > 0){
                return n;
            }

            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_receipttime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_receipttime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::READ);
                    }, weak_t, false);
                }
                int ret = scheduler->addEvent(m_sock, Event::READ);
            
                if(ret){
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    
                    if(*t != 0){
                        errno = *t;
                        LOG_DEBUG(logger_) << "timer timeout : " << errno;
                        return -1;
                    }

    
                }
            }
        }
        LOG_DEBUG(logger_) << "strerron : " << errno;
    }

    int Socket::recv(iovec* buffers, size_t length, int flags){
        if(!isConned()){
            return -1;
        }
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffers;
        msg.msg_iovlen = length;
        while(true){
            int n = ::recvmsg(m_sock, &msg, flags);

            while(n == -1 && errno == EINTR){
                n = ::recvmsg(m_sock, &msg, flags);
            }

            if(n > 0){
                return n;
            }

            if(n == -1 && errno == EWOULDBLOCK){
                
                std::shared_ptr<int> t(new int(0));
                std::weak_ptr<int> weak_t(t);
                Timer::ptr timer;
                int sock = m_sock;
                
                auto scheduler = m_scheduler;
                if(m_receipttime != (uint64_t)-1){
                    timer = m_scheduler->addConditionTimer(m_receipttime, [weak_t, scheduler, sock](){
                        auto i = weak_t.lock();
                        if(!i || *i){
                            return;
                        }
                        *i = ETIMEDOUT;
                        scheduler->cancelEvent(sock, Event::READ);
                    }, weak_t, false);
                }
                int ret = scheduler->addEvent(m_sock, Event::READ);

                if(ret){
                    
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }
                    return -1;
                } else {
                    Coroutine::ToSUSPENDED();
                    if(timer){
                        m_scheduler->cancelTimer(timer);
                    }

                    if(*t){
                        errno = *t;
                        return -1;
                    }
                }
            }

        }
    }

    IPv4Address::ptr Socket::getRemoteAddress(){
        if(m_remote){
            return m_remote;
        }

        IPv4Address::ptr ret(new IPv4Address());
        socklen_t addrlen = ret->getAddressLen();
        if(getpeername(m_sock, ret->getAddress(), &addrlen)){
            LOG_WARN(logger_) << "getRemoteAddress erron!! sock = " << m_sock; 
            return nullptr;
        }
        m_remote = ret;
        return m_remote;
    }

    IPv4Address::ptr Socket::getLocalAddress(){
        if(m_local){
            return m_local;
        }

        IPv4Address::ptr ret(new IPv4Address());
        socklen_t addrlen = ret->getAddressLen();
        if(getsockname(m_sock, ret->getAddress(), &addrlen)){
            LOG_WARN(logger_) << "getLocalAddress erron!! sock = " << m_sock; 
            LOG_ERROR(logger_) << "erron = "<< errno << " strerronr = " << strerror(errno);
            return nullptr;
        }
        m_local = ret;
        return m_local;
    }

    bool Socket::isConned() {
        return m_isConned;
    }

    bool Socket::isValid() const {
        return m_sock != -1;
    }



    int Socket::getError(){
        int error = 0;
        socklen_t len = sizeof(error);
        if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)){
            error = errno;
        }
        return error;
    }

    std::string Socket::toString() const {
        std::stringstream ss;
        ss  << "[Socket sock=" << m_sock
            << " is_connected=" << m_isConned
            << " family=" << m_family
            << " type=" << m_type
            << " protocol=" << m_protocol;
        if(m_local){
            ss << " local_address = " << m_local->toString();
        }
        if(m_remote){
            ss << " remote_address = " << m_remote->toString();
        }

        return ss.str();
    }

    bool Socket::init(int sock){
        m_sock = sock;
        m_isConned = true;
        if(m_sock < 0){
            
            LOG_ERROR(logger_) << "sock = " << sock << "init() fail!";
            return false;
        }
        setNonBlock(m_sock);
        initSocket();
        auto t = getLocalAddress();
        getRemoteAddress();
        if(t == nullptr){
            LOG_ERROR(logger_) << "sock = " << sock << "init() fail!";
            return false;
        }
        return true;
    }

    void Socket::newSocket(){
        m_sock = ::socket(m_family, m_type, m_protocol);
        if(m_sock != -1){
            initSocket();
        }
        if(m_sock == -1){
            LOG_ERROR(logger_) << "newSocket erron !! erron : " << strerror(errno);
            return;
        }

    }

    void Socket::initSocket(){
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if(m_type == SOCK_STREAM) {
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }

    bool Socket::cancelAccept(){
        return m_scheduler->cancelEvent(m_sock, Event::READ);
    }

    bool Socket::cancelWrite(){
        return m_scheduler->cancelEvent(m_sock, Event::WRITE);
    }

    bool Socket::cancelRead(){
        return m_scheduler->cancelEvent(m_sock, Event::READ);
    }

    bool Socket::cancelAll(){
        return m_scheduler->cancelAll(m_sock);
    }

    void Socket::deleteEventCb(){
        m_scheduler->deleteEventCb(m_sock);
    }
}