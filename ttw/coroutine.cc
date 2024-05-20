

#include "ioscheduler.h"
#include "coroutine.h"
#include "corscheduler.h"
#include <atomic>
#include "config.h"
#include "ttwassert.h"
#include "log.h"



constexpr size_t STACKSIZE = 128 * 1024;
static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

namespace ttw {

    
    static std::atomic<uint64_t> t_coroutine_id {0};
    static std::atomic<uint64_t> t_coroutine_count {0};

    static thread_local Coroutine* t_Coroutine = nullptr;
    static thread_local Coroutine::ptr t_sharedCoroutine = nullptr;

    Coroutine::Coroutine(){
        m_state = RUNNING;
        SetThis(this);

        if(getcontext(&m_ctx)){
            ASSERT2(false, "getcontext");
        }
        ++t_coroutine_count;
    }
    Coroutine::Coroutine(std::function<void()> cb, size_t stacksize):m_id(++t_coroutine_id), m_state(INIT), m_cb(cb){
        ++t_coroutine_count;

        m_stackSize = stacksize ? stacksize : STACKSIZE;

        m_stackBuf = malloc(m_stackSize);

        if(getcontext(&m_ctx)){
            LOG_ERROR(logger_) << "getcontext fail, coroutine id : " << m_id;
            ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stackBuf;
        m_ctx.uc_stack.ss_size = m_stackSize;
        
        makecontext(&m_ctx, Coroutine::Mian, 0);
    }

    Coroutine::~Coroutine(){
        --t_coroutine_count;
        if(m_stackBuf){

            free(m_stackBuf);
        }else {
            ASSERT(!m_cb);
            ASSERT(m_state == RUNNING);
            Coroutine* cur = t_Coroutine;
            if(cur == this){
                SetThis(nullptr);
            }
        }
        LOG_DEBUG(logger_) << "~Coroutine id = " << m_id;
    }
    
    void Coroutine::reset(std::function<void()> cb){
        ASSERT(m_stackBuf);
        ASSERT(m_state == INIT || m_state == FINIAH || m_state == EXCEPT);
        m_cb = cb;
        if(getcontext(&m_ctx)){
            ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stackBuf;
        m_ctx.uc_stack.ss_size = m_stackSize;
        makecontext(&m_ctx, Coroutine::Mian, 0);
        m_state = INIT;
    }





    void Coroutine::SetThis(Coroutine* val){
        t_Coroutine = val;
    }

    Coroutine::ptr Coroutine::GetCurCoroutine(){
        if(t_Coroutine){
            return t_Coroutine->shared_from_this();
        }

        Coroutine::ptr main_(new Coroutine);
        t_sharedCoroutine = main_;
        return t_Coroutine->shared_from_this();
    }

    void Coroutine::ToReady(){
        Coroutine::ptr cur = GetCurCoroutine();
        LOG_DEBUG(logger_) << cur->getState();
        ASSERT(cur->m_state == RUNNING);
        cur->m_state = READY;
        cur->swapOut();
    }

    void Coroutine::ToSUSPENDED(){
        Coroutine::ptr cur = GetCurCoroutine();
        auto rcur = cur.get();
        ASSERT(cur->m_state == RUNNING);
        //cur->m_state = SUSPENDED;
        rcur->swapOut();
    }

    void Coroutine::Mian(){
        Coroutine::ptr cur = GetCurCoroutine();
        ASSERT(cur);

       try{
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = FINIAH;
        }catch(std::exception& ex){
            cur->m_state = EXCEPT;
            LOG_ERROR(logger_) << "Coroutine Except : " << ex.what()
                << " Coroutine_id = " << cur->getCorId()
                << std::endl << ttw::BacktraceToString();
        }catch(...){
            cur->m_state = EXCEPT;
            LOG_ERROR(logger_) << "Coroutine Except" 
                << " Coroutine_id = " << cur->getCorId()
                << std::endl << ttw::BacktraceToString();
        }
        auto curp = cur.get();
        cur.reset();
        curp->swapOut();
    }

    void Coroutine::swapIn(){
        SetThis(this);
        ASSERT(m_state != RUNNING);
        m_state = RUNNING;
        if(swapcontext(&CorScheduler::GetMianCor()->m_ctx, &m_ctx)){
            ASSERT2(false, "swapcontext");
        }
    } 

    void Coroutine::swapOut(){
        SetThis(CorScheduler::GetMianCor());
        if(swapcontext(&m_ctx, &CorScheduler::GetMianCor()->m_ctx)){
            ASSERT2(false, "swapcontext");
        }
    }

    Coroutine::CoState Coroutine::getState(){
        return m_state;
    }

    uint64_t Coroutine::getCorId(){
        return m_id;
    }

    uint64_t Coroutine::GetCoroutineId(){
        return GetCurCoroutine()->m_id;
    }

    uint64_t Coroutine::GetCoroutineCount(){
        return t_coroutine_count;
    }

    void Coroutine::sleep(uint64_t time, IOScheduler* scheduler){
        auto cur = Coroutine::GetCurCoroutine();
        scheduler->addTimer(time, [scheduler, cur](){
            scheduler->run(cur);
        }, false);
        Coroutine::ToSUSPENDED();

    }

    void Coroutine::setState(Coroutine::CoState v){
        
    }
}
