
#ifndef __TTW_IOSCHEDELER_H
#define __TTW_IOSCHEDELER_H

#include<memory>
#include<map>
#include <shared_mutex>
#include "thread.h"
#include "nocopy.h"
#include "timer.h"
#include "corscheduler.h"

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
        ~FdContext();

        struct EventContext{
            std::function<void()> cb;
            Coroutine::ptr coroutine;
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);


        EventContext read;
        EventContext write;

        int fd = 0;

        Event events = NONE;

        std::mutex m_mutex;

    };

    class IOScheduler : public  TimerManage, public CorScheduler, std::enable_shared_from_this<IOScheduler>{
    public:
        using ptr = std::shared_ptr<IOScheduler>;
        IOScheduler(const std::string& name, int threads = 1, bool stop = false);
        virtual ~IOScheduler();

        int addEvent(int fd, Event eevent, std::function<void()> cb = nullptr);


        bool delEvent(int fd, Event event);

        bool cancelEvent(int fd, Event event);
        
        bool cancelAll(int fd);

        void run(std::function<void()> cb);
        void run(Coroutine::ptr val);


        void deleteEventCb(int fd);

        int getEpollFd() const {
            return m_epfd;
        }

        virtual void triggerEvent(FdContext* text, Event event);

        void notify();
        void wait() override;
        bool is_stop();
        bool is_stop(uint64_t& timeout);
        //void stop();
        virtual void init(int size);

        //void start();

        //void join();
    protected:
        void notifyTimer() override;
        void notifyCor() override;
    protected:
        std::string m_name;
        int m_threads;
        int m_epfd;
        int m_pipe[2];
        std::atomic<int>  m_eventConut = {0};
        std::shared_mutex m_rwmutex;
        std::map<int, FdContext*> m_fdContexts;
        std::thread m_rootThread;
        bool m_stop;
    }; 
    
}


#endif