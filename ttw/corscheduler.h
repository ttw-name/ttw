

#ifndef __TTW__CORSCHEDULER_H
#define __TTW__CORSCHEDULER_H

#include "coroutine.h"


#include <thread>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <string>
#include <atomic>
#include <vector>
#include <list>
#include <functional>



namespace ttw {
    class CorScheduler : public std::enable_shared_from_this<CorScheduler>{
    friend class Coroutine;
    public:
        using ptr = std::shared_ptr<CorScheduler>;

        CorScheduler(const std::string& name, int threads);

        ~CorScheduler();

        void add(std::function<void()> cb);

        void add(Coroutine::ptr cor);

        void run();

        virtual void wait() = 0;

        void start();

        void stop();

        virtual void notifyCor();

        void join();

        static Coroutine* GetMianCor();

    private:

        std::string m_name;

        std::vector<std::thread> m_threads;

        std::list<Coroutine::ptr> m_coroutines;

        std::mutex m_mutex;

        size_t m_threadCount;

        std::atomic<size_t> m_workThreadsCount = {0};

        std::atomic<size_t> m_idleThreadsCount = {0};

        bool m_stop;
    }; 
};

#endif