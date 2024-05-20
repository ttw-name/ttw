#include "timer.h"
#include "util.h"
#include "config.h"
namespace ttw {


   Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring)
   :m_ms(ms), m_cb(cb), m_recurr(recurring) {
        m_next = ttw::GetCurrentMs() + ms;
   } 

   Timer::Timer(uint64_t ms):m_next(ms){

   }

   bool Timer::Comp::operator()(const Timer::ptr& l, const Timer::ptr& r) const {
        if(!l && !r){
            return false;
        }

        if(!l){
            return true;
        }

        if(!r){
            return false;
        }

        if(l->m_next < r->m_next){
            return true;
        }

        if(r->m_next < l->m_next){
            return false;
        }

        return l.get() < r.get();
   }



   bool TimerManage::cancelTimer(Timer::ptr val){
        std::unique_lock<std::shared_mutex> wlock(m_rwlock);
        auto it = m_timers.find(val);
        if(it != m_timers.end()){
            m_timers.erase(it);
            return true;
        }
        return false;
   }

   bool TimerManage::refreshTimer(Timer::ptr val){
        std::unique_lock<std::shared_mutex> wlock(m_rwlock);
        auto it = m_timers.find(val);
        if(it == m_timers.end()){
            return false;
        }
        m_timers.erase(it);
        val->m_next = ttw::GetCurrentMs() + val->m_ms;
        m_timers.insert(val);
        return true;
   }

   bool TimerManage::resetTimer(Timer::ptr val, uint64_t ms, bool from_now){
        if(ms == val->m_ms && !from_now)
            return true;
        {
            std::unique_lock<std::shared_mutex> wlock(m_rwlock);
            auto it = m_timers.find(val);
            if (it == m_timers.end())
            {
                return false;
            }
            m_timers.erase(it);

            uint64_t t = 0;
            if (from_now)
            {
                t = ttw::GetCurrentMs();
            }
            else
            {
                t = val->m_next - val->m_ms;
            }
            val->m_ms = ms;
            val->m_next = t + ms;
        }
        addTimer(val);
        
        return true;
    }

    TimerManage::TimerManage(){
        m_preTime = ttw::GetCurrentMs();
    }

    TimerManage::~TimerManage(){

    }

    Timer::ptr TimerManage::addTimer(uint64_t ms, std::function<void()> cb, bool recurring){
        Timer::ptr timer(new Timer(ms, cb, recurring));
        addTimer(timer);
        return timer;
    }

    void TimerManage::addTimer(Timer::ptr val){
        bool at_front = false;
        {
            std::unique_lock<std::shared_mutex> wlock(m_rwlock);
            auto it = m_timers.insert(val).first;
            at_front = (it == m_timers.begin());
        }
        if(at_front){
            notifyTimer();
        }
    }

    static void runTimer(std::weak_ptr<void> weak_p, std::function<void()> cb){
        std::shared_ptr<void> ptr = weak_p.lock();
        if(ptr)
            cb();
    }

    Timer::ptr TimerManage::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_p, bool recurring){
        return addTimer(ms, std::bind(&runTimer, weak_p, cb), recurring);
    }

    uint64_t TimerManage::getNextTimer(){
        std::shared_lock<std::shared_mutex> rlock(m_rwlock);

        if(m_timers.empty()){
            return ~0ull;
        }

        const Timer::ptr& next = *m_timers.begin();
        uint64_t now = ttw::GetCurrentMs();
        if(now >= next->m_next){
            return 0;
        }else{
            return next->m_next - now;
        }
    }

    void TimerManage::listExpiredCbs(std::vector<std::function<void()>> &cbs){
        uint64_t now = ttw::GetCurrentMs();
        std::vector<Timer::ptr> expired;
        {
            std::shared_lock<std::shared_mutex> rlock(m_rwlock);
            if(m_timers.empty()){
                return;
            }
            
            if((*m_timers.begin())->m_next > now){
                return;
            }
        }

        std::unique_lock<std::shared_mutex> wlock(m_rwlock);
        Timer::ptr now_timer(new Timer(now));
        auto it = m_timers.lower_bound(now_timer);
        while(it != m_timers.end() && (*it)->m_next == now){
            ++it;
        }
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.resize(expired.size());

        for(auto& timer : expired){
            cbs.push_back(timer->m_cb);
            if(timer->m_recurr){
                timer->m_next = now + timer->m_ms;
                m_timers.insert(timer);
            }
        }
    }

    bool TimerManage::emptyTimer(){
        std::shared_lock<std::shared_mutex> rlock(m_rwlock);
        return m_timers.empty();
    }

}