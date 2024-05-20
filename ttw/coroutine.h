#ifndef __TTW__COROUTINE_H
#define __TTW__COROUTINE_H



#include <ucontext.h>
#include <memory>
#include <functional>
#include <queue>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>



namespace ttw {
    class CorScheduler;
    class IOScheduler;
    class Coroutine :public std::enable_shared_from_this<Coroutine>{
    friend class CorScheduler;
    public:
        using ptr = std::shared_ptr<Coroutine>;
        enum CoState{
            INIT,
            READY,
            RUNNING,
            SUSPENDED,
            FINIAH,
            EXCEPT
        };

        Coroutine();
        Coroutine(std::function<void()> cb, size_t stacksize = 0);
        ~Coroutine();

        void reset(std::function<void()> cb = nullptr);

        static Coroutine::ptr GetCurCoroutine();

        static uint64_t GetCoroutineId();

        static uint64_t GetCoroutineCount();

        static void Mian();

        static void SetThis(Coroutine* ptr);

        static void ToReady();
        static void ToSUSPENDED();



        
        static void sleep(uint64_t time, IOScheduler* scheduler);



        void swapIn();
        void swapOut();

        uint64_t getCorId(); 

        CoState getState();

        void setState(CoState v);

    private:
        uint64_t m_id;
        uint32_t m_stackSize;

        CoState m_state;
        ucontext_t m_ctx;

        void* m_stackBuf;
        std::function<void()> m_cb;
        
    };
}

#endif