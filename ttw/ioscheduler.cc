
#include<fcntl.h>
#include<errno.h>
#include<sys/epoll.h>
#include <chrono>
#include <functional>

#include"config.h"
#include "log.h"
#include "ioscheduler.h"



static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw {
    
    FdContext::EventContext& FdContext::getContext(Event event){
        if(event == Event::READ)
            return read;
        
        if(event == Event::WRITE)
            return write;
        LOG_ERROR(logger_) << "getContext is error!";
        throw std::invalid_argument("");
    }

    void FdContext::resetContext(EventContext& ctx){
        ctx.cb = nullptr;
        ctx.coroutine.reset();
    }

    FdContext::~FdContext(){
            read.coroutine.reset();
            write.coroutine.reset();
            
    }

    IOScheduler::IOScheduler(const std::string& name, int threads, bool stop):CorScheduler(name, threads), m_name(name), m_threads(threads), m_stop(stop){
        m_epfd =  epoll_create(1);
        if(m_epfd < 0){
            LOG_ERROR(logger_) << "epoll_create return error!!";
        }

        int ret = pipe(m_pipe);
        if(ret < 0){
            LOG_ERROR(logger_) << "pipe call error!!";
        }

        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_pipe[0];

        ret = fcntl(m_pipe[0], F_SETFL, O_NONBLOCK);

        if(ret < 0){
            LOG_ERROR(logger_) << "fcntl call error!!";
        }

        ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_pipe[0], &event);
        if(ret < 0){
            LOG_ERROR(logger_) << "epoll_ctl call error!!";
        }
        
        //start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_DEBUG(logger_) << "IOScheduler cread successful! name  : " << m_name;

    }

    IOScheduler::~IOScheduler(){
        
        close(m_epfd);
        close(m_pipe[0]);
        close(m_pipe[1]);
        LOG_DEBUG(logger_) << "~IOScheuler() mane : " << m_name;
    }

    void IOScheduler::run(std::function<void()> cb){
        add(cb);
        
    }
    void IOScheduler::run(Coroutine::ptr val){
        add(val);
    }

    // void IOScheduler::start(){
    //     init(m_threads);
    //     m_rootThread = std::thread([this]{ this->wait(); });
    //     //m_coroutineScheduler->add([this]{ this->wait(); });
    //     m_coroutineScheduler->start();
    //     //t->join();
    // }

    // void IOScheduler::join(){
    //     join();
    // }

    int IOScheduler::addEvent(int fd, Event event, std::function<void()> cb){
        
        FdContext* fd_ctext = nullptr;
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            if(m_fdContexts.find(fd) != m_fdContexts.end()){
                fd_ctext = m_fdContexts[fd];
            }else{
                r_lock.unlock();
                std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
                fd_ctext = new FdContext;
                m_fdContexts[fd] = fd_ctext;
            }

        }
        
        {
            std::unique_lock<std::mutex> fdlock(fd_ctext->m_mutex);
            fd_ctext->fd = fd;
            if(fd_ctext->events & event){
                LOG_ERROR(logger_) << "epoll event is exist!";
                throw std::invalid_argument("");
            }
            int op = fd_ctext->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
            epoll_event ep_event;
            memset(&ep_event, 0, sizeof(ep_event));
            ep_event.events = EPOLLET | fd_ctext->events | event;
            ep_event.data.ptr = fd_ctext;

            int ret = epoll_ctl(m_epfd, op, fd, &ep_event);
            if(ret){
                LOG_ERROR(logger_) << "epoll_ctl fail, fd = " << fd;
                return ret;
            }

            ++m_eventConut;
            fd_ctext->events = (Event)(fd_ctext->events | event);
            FdContext::EventContext& event_ctext = fd_ctext->getContext(event);

            if(cb){
                event_ctext.cb.swap(cb);
            }else{
                event_ctext.coroutine = Coroutine::GetCurCoroutine();
            }
        }
        LOG_DEBUG(logger_) << "addEeven end";
        return 0;
    }

    bool IOScheduler::delEvent(int fd, Event event){
        
        FdContext* fd_ctext = nullptr;
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            if(m_fdContexts.find(fd) == m_fdContexts.end())
                return false;
            fd_ctext = m_fdContexts[fd];
        }

        {
            std::unique_lock<std::mutex> fd_lock(fd_ctext->m_mutex);
            if(!(fd_ctext->events & event))
                return false;
            Event delevent = (Event)(fd_ctext->events & ~event);
            int op = delevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            epoll_event ep_event;
            memset(&ep_event, 0, sizeof(ep_event));
            ep_event.events = delevent | EPOLLET;
            ep_event.data.ptr = fd_ctext;

            int ret = epoll_ctl(m_epfd, op, fd, &ep_event);
            if(ret){
                LOG_ERROR(logger_) << "epoll_ctl fail, id  = " << fd;
                return false;
            }

            --m_eventConut;
            fd_ctext->events = delevent;
            FdContext::EventContext& event_text = fd_ctext->getContext(event);
            fd_ctext->resetContext(event_text);

            if(fd_ctext->events == NONE){
                deleteEventCb(fd);
            }
            return true;
        }

    }

    bool IOScheduler::cancelEvent(int fd, Event event){
        
        FdContext* fd_text = nullptr;
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            if(m_fdContexts.find(fd) == m_fdContexts.end())
                return false;
            fd_text = m_fdContexts[fd];
        }

        {
            std::unique_lock<std::mutex> fd_lock(fd_text->m_mutex);
            if(!(fd_text->events & event)){
                return false;
            }

            Event cancelevent = (Event)(fd_text->events & ~event);
            int op = cancelevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            epoll_event ep_event;
            memset(&ep_event, 0, sizeof(ep_event));
            ep_event.events = cancelevent | EPOLLET;
            ep_event.data.ptr = fd_text;

            int ret = epoll_ctl(m_epfd, op, fd, &ep_event);
            if(ret){
                LOG_ERROR(logger_) << "epoll_ctl fail, fd = " << fd;
                return false;
            }
            triggerEvent(fd_text, event);
            --m_eventConut;

            if(fd_text->events == NONE){
                
                deleteEventCb(fd);
            }

            return true;

        }
        
    }

    bool IOScheduler::cancelAll(int fd){
        FdContext* fd_text = nullptr;
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            if(m_fdContexts.find(fd) == m_fdContexts.end())
                return false;
            fd_text = m_fdContexts[fd];
        }

        {
            std::unique_lock<std::mutex> fd_lock(fd_text->m_mutex);
            if(!(fd_text->events)){
                return false;
            }

            int op = EPOLL_CTL_DEL;
            epoll_event ep_event;
            memset(&ep_event, 0, sizeof(ep_event));
            ep_event.events = EPOLLET;
            ep_event.data.ptr = fd_text;

            int ret = epoll_ctl(m_epfd, op, fd, &ep_event);
            if(ret){
                LOG_ERROR(logger_) << " ";
                return false;
            }
            if(fd_text->events & READ){
                triggerEvent(fd_text, READ);
                --m_eventConut;
            }

            if(fd_text->events & READ){
                triggerEvent(fd_text, READ);
                --m_eventConut;
            }
            deleteEventCb(fd);

            return true;

        }
    }

    void IOScheduler::deleteEventCb(int fd){
       
        {
            std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
            auto it = m_fdContexts.find(fd);
        
            if(it == m_fdContexts.end()){
                return;
            }
            delete it->second;
            m_fdContexts.erase(it);            
        }
    }

    void IOScheduler::wait(){

        epoll_event* events = new epoll_event[MAX_EPOLL_EVENTS]();
        std::shared_ptr<epoll_event> ptr_event(events, [](epoll_event* ptr){ delete[] ptr; });

        while(true){
            uint64_t next_timeout = 0;
            if(is_stop(next_timeout)){
                LOG_DEBUG(logger_) << "IOScheduler wait stop, name : " << m_name;
                break;
            }

            int ret = 0;
            do {
                if(next_timeout != ~0ull){
                    next_timeout = (int) next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;

                } else {
                    next_timeout = MAX_TIMEOUT;
                }
                ret = epoll_wait(m_epfd, events, MAX_EPOLL_EVENTS, (int)next_timeout);

                if(ret < 0 && errno == EINTR){

                }else{
                    break;
                }
            }while(true);

            std::vector<std::function<void()>> cbs;
            listExpiredCbs(cbs);
            if(!cbs.empty()){
                for(auto& cb : cbs){
                    add(cb);
                }
                cbs.clear();
            }

            for(int i = 0; i < ret; ++i){
                epoll_event& event = events[i];
                if(event.data.fd == m_pipe[0]){
                    char dummy[256];
                    while(read(m_pipe[0], dummy, sizeof dummy) > 0);
                    continue;
                }

                FdContext* fd_text = (FdContext* )event.data.ptr;
                std::unique_lock<std::mutex> fd_lock(fd_text->m_mutex);
                if(event.events & (EPOLLERR | EPOLLHUP)){
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_text->events;
                }

                int real = NONE;
                if(event.events & EPOLLIN){
                    real |= READ;
                }

                if(event.events & EPOLLOUT){
                    real |= WRITE;
                }

                if((fd_text->events & real) == NONE){
                    continue;
                }

                int prv_events = (fd_text->events & ~real);
                int op = prv_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | prv_events;

                int ret1 = epoll_ctl(m_epfd, op, fd_text->fd, &event);
                if(ret1){
                    LOG_ERROR(logger_) << "epoll_ctl error!! erron: " << strerror(errno) << "fd : " << fd_text->fd;
                    continue;
                }

                if(real & READ){
                    triggerEvent(fd_text, READ);
                    --m_eventConut;
                }

                if(real & WRITE){
                    triggerEvent(fd_text, WRITE);
                    --m_eventConut;
                }

                if(op == EPOLL_CTL_DEL){
                    deleteEventCb(fd_text->fd);
                }

                //LOG_DEBUG(logger_) << "wait end!!";
                //std::this_thread::sleep_for(std::chrono::milliseconds(7000));
            }

            Coroutine::ptr cur = Coroutine::GetCurCoroutine();
            auto raw_ptr = cur.get();
            cur.reset();

            raw_ptr->swapOut();
            
        }
    }

    void IOScheduler::notifyTimer(){
        notify();
    }

    void IOScheduler::notifyCor(){
        notify();
    }

    void IOScheduler::notify(){
        int ret = write(m_pipe[1], "n", 1);
        if(ret != 1){
            LOG_ERROR(logger_) << "write call fuial!!";
            throw std::bad_function_call();
        }
    }
    
    void IOScheduler::triggerEvent(FdContext* fd_text, Event event){
        fd_text->events = (Event)(fd_text->events & ~event);
        switch (event)
        {
        case READ:{
                auto f = fd_text->getContext(event);
                if(f.coroutine){
                    add(f.coroutine);
                }

                if(f.cb){
                    add(f.cb);
                }
                break;
            }
        case WRITE:{
                auto f = fd_text->getContext(event);
                if(f.coroutine){
                    add(f.coroutine);
                }

                if(f.cb){
                    add(f.cb);
                }
                break;
            }
        default:
            break;
        }
    }

    bool IOScheduler::is_stop(){
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        return m_stop && m_eventConut;
    }

    bool IOScheduler::is_stop(uint64_t& timeout){
        timeout = getNextTimer();
        return timeout == ~0ull && is_stop();
    }
    
    // void IOScheduler::stop(){
    //     std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
    //     m_stop = true;
    //     if(!){
    //         return;
    //     }
    //     m_coroutineScheduler->stop();
    // }

    void IOScheduler::init(int size){
            // {
            //     std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
            //     m_coroutineScheduler.reset(new CorScheduler(m_name, size));
            // }
    }



}