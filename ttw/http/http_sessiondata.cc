
#include "http_sessiondata.h"
#include "ttw/util.h"
#include <strstream>
#include <sstream>
namespace ttw {
namespace http{


    SessionData::SessionData(){
        m_time = GetCurrentMs();
    }

    void SessionData::addDate(const std::string& key, boost::any val){
        
        if(m_data.find(key) == m_data.end()){
            m_data[key] = val;
        }
    }

    void SessionData::delDate(const std::string& key){
        m_data.erase(key);
    }

    void SessionData::addDate(std::vector<std::pair<std::string, boost::any>>& datas){
        for(auto& it : datas){
            addDate(it.first, it.second);
        }
    }

    void SessionData::setDate(const std::string& key, boost::any val){
        m_data[key] = val;
    }

    void SessionData::refreshTime(){
        m_time = GetCurrentMs();
    }

    void SessionData::refreshTime(uint64_t now){
        m_time = now;
    }

    bool SessionData::isValid(uint64_t time){
        uint64_t now = GetCurrentMs();
        if(now - m_time <= time){
            refreshTime(now);
            return true;
        }
        return false;
    }

    std::string SessionData::toString(){
        std::unique_lock<std::mutex> lock(m_mutex);
        std::stringstream ss;
        ss << "{ "; 
        for(auto it : m_data){
            ss << it.first << " : ";
            ss <<  boost::any_cast<std::string>(it.second) << ", ";  
        }
        ss << " }";
        return ss.str();
    }

    SessionDataMan::SessionDataMan(IOScheduler::ptr scheduler, uint64_t timertime):m_ttime(timertime) {
        scheduler->addTimer(m_ttime, std::bind(&SessionDataMan::timer, this), true);
    }


    bool SessionDataMan::hasSessionData(const std::string& key){
        
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        auto it = m_sessionDataMap.find(key);
        if(it == m_sessionDataMap.end()){
            return false;
        }

        std::unique_lock<std::mutex> lock(it->second->m_mutex);
        if(!it->second->isValid(m_ttime)){
            return false;
        }
        return true;
    }

    SessionData::ptr SessionDataMan::getSessionData(const std::string& key){
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        auto it = m_sessionDataMap.find(key);
        if(it != m_sessionDataMap.end()){
            std::unique_lock<std::mutex> lock(it->second->m_mutex);

            if(!it->second->isValid(m_ttime)){
                return nullptr;
            }
            return it->second;
        }else{
            return nullptr;
        }
    }
    void SessionDataMan::delSessionData(const std::string& key){
        std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
        m_sessionDataMap.erase(key);
    }
    void SessionDataMan::setSessionData(const std::string& key, std::vector<std::pair<std::string, boost::any>>& data){
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        auto it = m_sessionDataMap.find(key);
        if(it != m_sessionDataMap.end()){
            std::unique_lock<std::mutex> lock(it->second->m_mutex);
            for(auto& i : data){
                it->second->setDate(i.first, i.second);
            }
        }
    }

    void SessionDataMan::setSessionData(const std::string& key, std::string key1, boost::any any){
        std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
        auto it = m_sessionDataMap.find(key);
        if(it != m_sessionDataMap.end()){
            std::unique_lock<std::mutex> lock(it->second->m_mutex);
            
            it->second->setDate(key1, any);
            
        }
    }

    void SessionDataMan::addSessionData(const std::string& key){
        SessionData::ptr t(new SessionData);
        std::unique_lock<std::shared_mutex> w_lock(m_rwmutex);
        m_sessionDataMap[key] = t;
    }
    void SessionDataMan::timer(){
        uint64_t now = GetCurrentMs();
        std::vector<std::string> overtimes;
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            for(auto it : m_sessionDataMap){
                if(now - it.second->m_time > m_ttime){
                    overtimes.push_back(it.first);
                }
            }
        }
        for(auto& i : overtimes){
            delSessionData(i);
        }
    }

}
}