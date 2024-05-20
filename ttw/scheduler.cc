
#include<fcntl.h>
#include<errno.h>
#include<sys/epoll.h>
#include <chrono>

#include"config.h"
#include "log.h"
#include "scheduler.h"



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
    }

    void FdContext::triggerEvent(Event event){

    }

    Scheduler::Scheduler(const std::string& name, bool stop, int threads):m_name(name), m_stop(stop) {
        m_epfd = epoll_create(1);
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
        init(threads);
        m_pools->getThreadPool("root")->add([this](){this->wait();});
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_DEBUG(logger_) << "scheduler cread su!";

    }

    Scheduler::~Scheduler(){
        
        close(m_epfd);
        close(m_pipe[0]);
        close(m_pipe[1]);
        delete m_pools;
    }

    void Scheduler::start(){
        auto t = m_pools->getThreadPool("root");
        t->join();
    }

    int Scheduler::addEvent(int fd, Event event, std::function<void()> cb){
        
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
                LOG_ERROR(logger_) << "erron";
            }

            ++m_eventConut;
            fd_ctext->events = (Event)(fd_ctext->events | event);
            FdContext::EventContext& event_ctext = fd_ctext->getContext(event);

            if(cb){
                event_ctext.cb.swap(cb);
            }
            LOG_DEBUG(logger_) << "addEvent end";
        }
        return 0;
    }

    bool Scheduler::delEvent(int fd, Event event){
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
                LOG_ERROR(logger_) << "epoll_ctl erron ";
            }

            --m_eventConut;
            fd_ctext->events = delevent;
            FdContext::EventContext& event_text = fd_ctext->getContext(event);
            fd_ctext->resetContext(event_text);

            if(fd_ctext->events == NONE){
                std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
                auto it = m_fdContexts.find(fd);
                m_fdContexts.erase(it);
            }
            return true;
        }

    }

    bool Scheduler::cancelEvent(int fd, Event event){
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
                LOG_ERROR(logger_) << " ";
            }
            triggerEvent(fd_text, event);
            --m_eventConut;

            if(fd_text->events == NONE){
                std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
                auto it = m_fdContexts.find(fd);
                m_fdContexts.erase(it);
            }
            return true;

        }
        
    }

    bool Scheduler::cancelAll(int fd){

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
            }
            if(fd_text->events & READ){
                triggerEvent(fd_text, READ);
                --m_eventConut;
            }

            if(fd_text->events & READ){
                triggerEvent(fd_text, READ);
                --m_eventConut;
            }
            if(fd_text->events == NONE){
                std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
                auto it = m_fdContexts.find(fd);
                m_fdContexts.erase(it);
            }
            return true;

        }
    }

    void Scheduler::wait(){

        epoll_event* events = new epoll_event[MAX_EPOLL_EVENTS]();
        std::shared_ptr<epoll_event> ptr_event(events, [](epoll_event* ptr){ delete[] ptr; });

        while(true){
            uint64_t next_timeout = 0;
            if(is_stop(next_timeout)){
                LOG_DEBUG(logger_) << "wait stop";
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
                    //LOG_DEBUG(logger_) << "epoll_wait break";
                    break;
                }
            }while(true);

            std::vector<std::function<void()>> cbs;
            listExpiredCbs(cbs);
            if(!cbs.empty()){
                for(auto& cb : cbs){
                    m_pools->getThreadPool("timer")->add(cb);
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
                    LOG_DEBUG(logger_) << "READ";
                    real |= READ;
                }

                if(event.events & EPOLLOUT){
                    LOG_DEBUG(logger_) << "WRITE";
                    real |= WRITE;
                }

                if((fd_text->events & real) == NONE){
                    continue;
                }

                int prv_events = (fd_text->events & ~real);
                LOG_DEBUG(logger_) << "events: " << prv_events; 
                int op = prv_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | prv_events;

                int ret1 = epoll_ctl(m_epfd, op, fd_text->fd, &event);
                if(ret1){
                    LOG_ERROR(logger_) << "epoll_ctl error!! erron: " << strerror(errno);
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
                    LOG_INFO(logger_) << "delete fd: " << fd_text->fd;
                    m_fdContexts.erase(fd_text->fd);
                }

                //LOG_DEBUG(logger_) << "wait end!!";
                //std::this_thread::sleep_for(std::chrono::milliseconds(7000));
            }
        }
    }

    void Scheduler::notifyTimer(){
        notify();
    }

    void Scheduler::notify(){
        int ret = write(m_pipe[1], "n", 1);
        if(ret != 1){
            LOG_ERROR(logger_) << "write call fuial!!";
            throw std::bad_function_call();
        }
    }
    
    void Scheduler::triggerEvent(FdContext* fd_text, Event event){
        fd_text->events = (Event)(fd_text->events & ~event);
        switch (event)
        {
        case READ:{
                auto f = fd_text->getContext(event);
                auto pool = m_pools->getThreadPool("read");
                pool->add(f.cb);
                break;
            }
        case WRITE:{
                auto f = fd_text->getContext(event);
                auto pool = m_pools->getThreadPool("write");
                pool->add(f.cb);
                break;
            }
        default:
            break;
        }
    }

    bool Scheduler::is_stop(){
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        return m_stop && m_eventConut;
    }

    bool Scheduler::is_stop(uint64_t& timeout){
        timeout = getNextTimer();
        return timeout == ~0ull && is_stop();
    }
    
    void Scheduler::stop(){
        std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
        m_stop = true;
        if(!m_pools){
            return;
        }
        m_pools->stop();
    }

    void Scheduler::init(int size){
            {
                std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
                m_pools = new ThreadPoolMan(false);
                m_pools->addThreadPool(std::shared_ptr<ThreadPool>(new ThreadPool("read", size)));
                m_pools->addThreadPool(std::shared_ptr<ThreadPool>(new ThreadPool("write", size)));
                m_pools->addThreadPool(std::shared_ptr<ThreadPool>(new ThreadPool("timer", size)));
                m_pools->addThreadPool(std::shared_ptr<ThreadPool>(new ThreadPool("root", 1)));
            }
    }



}