#ifndef __TTW_SCHEDELER_H
#define __TTW_SCHEDELER_H

#include<memory>
#include<map>
#include<shared_mutex>
#include "thread.h"
#include "nocopy.h"
#include "timer.h"

#define MAX_EPOLL_EVENTS 256
#define MAX_TIMEOUT 3000


namespace ttw {
   
    

    enum Event{
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4,
    };
    
    class FdContext{
    public:

        struct EventContext{
            std::function<void()> cb;
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        EventContext read;
        EventContext write;

        int fd = 0;

        Event events = NONE;

        std::mutex m_mutex;

    };

    class Scheduler : public  TimerManage{
    public:
        using ptr = std::shared_ptr<Scheduler>;
        Scheduler(const std::string& name, bool stop, int threads = 1);
        ~Scheduler();

        int addEvent(int fd, Event eevent, std::function<void()> cb);

        bool delEvent(int fd, Event event);

        bool cancelEvent(int fd, Event event);
        
        bool cancelAll(int fd);

        int getEpollFd() const {
            return m_epfd;
        }

        virtual void triggerEvent(FdContext* text, Event event);

        void notify();
        virtual void wait();
        bool is_stop();
        bool is_stop(uint64_t& timeout);
        void stop();
        virtual void init(int size);

        void start();
    protected:
        void notifyTimer() override;

    protected:
        std::string m_name;
        int m_epfd;
        int m_pipe[2];
        std::atomic<int>  m_eventConut = {0};
        std::shared_mutex m_rwmutex;
        std::map<int, FdContext*> m_fdContexts;
        ThreadPoolMan* m_pools;
        bool m_stop;
    }; 
    
}


#endif