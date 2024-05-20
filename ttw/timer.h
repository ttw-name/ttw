#ifndef __TTW__TIMER_H
#define __TTW__TIMER_H

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <set>
#include <functional>

namespace ttw {
    class Timer{
        friend class TimerManage;
    public:
        using ptr = std::shared_ptr<Timer>;
        Timer(uint64_t ms, std::function<void()> cb, bool recurring);
        Timer(uint64_t ms);
    private:
        struct Comp{
            bool operator()(const Timer::ptr& l, const Timer::ptr& r) const;
        };
    private:
        uint64_t m_ms = 0;
        std::function<void()> m_cb;
        bool m_recurr = false;
        uint64_t m_next = 0;

    };

    class TimerManage {
    public:
        TimerManage();
        ~TimerManage();
        void addTimer(Timer::ptr val);
        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring);
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_p, bool recurring);
        uint64_t getNextTimer();

        void listExpiredCbs(std::vector<std::function<void()>> &cbs);
        //void onTimerInsertAtFront();
        bool emptyTimer();
        

        bool cancelTimer(Timer::ptr val);
        bool refreshTimer(Timer::ptr val);
        bool resetTimer(Timer::ptr val, uint64_t ms, bool from_now);

    protected:
        virtual void notifyTimer() = 0;
    private:
        std::shared_mutex m_rwlock;
        std::set<Timer::ptr, Timer::Comp> m_timers;
        uint64_t m_preTime;

    };
}


#endif