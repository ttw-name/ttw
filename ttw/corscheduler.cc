#include "corscheduler.h"
#include "config.h"
#include "ttwassert.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

namespace ttw {

    static thread_local Coroutine* t_corsheduler = nullptr;

    CorScheduler::CorScheduler(const std::string& name, int threads):m_name(name){
        m_threadCount = threads > 0 ? threads : std::thread::hardware_concurrency();
        m_stop = false;
        ASSERT(m_threadCount != 0);
    }

    CorScheduler::~CorScheduler(){
        
    }

    Coroutine* CorScheduler::GetMianCor(){
        return t_corsheduler;
    }

    void CorScheduler::run(){
        t_corsheduler = Coroutine::GetCurCoroutine().get();
        
        Coroutine::ptr idleCor(new Coroutine(std::bind(&CorScheduler::wait, this)));
        Coroutine::ptr cor;

        while(true){
            cor.reset();
            bool notifyme = false;
            bool is_work = false;
            LOG_DEBUG(logger_) << "Coroutine count = " << Coroutine::GetCoroutineCount();
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                auto it = m_coroutines.begin();
                while(it != m_coroutines.end()){

                    if(*it == nullptr){
                        m_coroutines.erase(it);
                        ++it;
                        continue;
                    }

                    if(*it && (*it)->getState() == Coroutine::RUNNING){
                        ++it;
                        continue;
                    }

                    cor = *it;
                    m_coroutines.erase(it);
                    ++m_workThreadsCount;
                    is_work = true;
                    break;
                }
                notifyme |= it != m_coroutines.end();

            }
            if(notifyme){
                notifyCor();
            }

            if(cor && cor->getState() != Coroutine::FINIAH && cor->getState() != Coroutine::EXCEPT){

                cor->swapIn();

                --m_workThreadsCount;  
                if(cor->getState() == Coroutine::READY){
                    add(cor);
                }else if(cor->getState() != Coroutine::FINIAH && cor->getState() != Coroutine::EXCEPT){
                    cor->m_state = Coroutine::SUSPENDED;
                }
                cor.reset();
            }else{
                if(is_work){
                    --m_workThreadsCount;
                    continue;
                }
                ++m_idleThreadsCount;
                idleCor->swapIn();
                --m_idleThreadsCount;
                if(idleCor->getState() != Coroutine::FINIAH
                    && idleCor->getState() != Coroutine::EXCEPT) {
                    idleCor->m_state = Coroutine::SUSPENDED;
                }

            }

        }

    }

    void CorScheduler::start(){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = false;

            m_threads.resize(m_threadCount);
        }
            for(size_t i = 0; i < m_threadCount; ++i){
                m_threads[i] = std::thread(std::bind(&CorScheduler::run, this));
            }
        
    }

    void CorScheduler::stop(){
        
       
       m_stop = true;

        for(size_t i = 0; i < m_threadCount; ++i){
            notifyCor();
        }
        for(auto& i : m_threads){
            i.join();
        }
         
    }

    void CorScheduler::notifyCor(){
        LOG_DEBUG(logger_) << "notify Cor";
    }

    void CorScheduler::wait(){
        LOG_DEBUG(logger_) << "wait";
    }


    void CorScheduler::add(std::function<void()> cb){
        if(cb == nullptr)
            return;
        Coroutine::ptr val(new Coroutine(cb));
        add(val);

    }

    void CorScheduler::add(Coroutine::ptr val){
        
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_coroutines.push_back(val);
        }
        notifyCor();
    }

    void CorScheduler::join(){
        for(auto& t : m_threads){
            t.join();
        }
    }
}
