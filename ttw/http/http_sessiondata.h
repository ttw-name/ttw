

#ifndef __TTW__HTTP__HTTP_SESSIONDATA_H
#define __TTW__HTTP__HTTP_SESSIONDATA_H


#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <boost/any.hpp>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <list>
#include "ttw/ioscheduler.h"

namespace ttw{
namespace http{
    class SessionDataMan;
    class SessionData : public std::enable_shared_from_this<SessionData> {
    friend SessionDataMan;
    public:
        using ptr = std::shared_ptr<SessionData>;
        SessionData();
        
        template <typename T>
        T getData(const std::string& key, T def){
            auto it = m_data.find(key);
            if(it == m_data.end()){
                return def;
            }
            try{
                return boost::any_cast<T>(it->second);
            }catch(...){
                return def;
            }
            return def;
        }

        std::string toString();
        
    
        void addDate(const std::string& key, boost::any val);
        void setDate(const std::string& key, boost::any val);
        void delDate(const std::string& key);
        void addDate(std::vector<std::pair<std::string, boost::any>>& datas);
        void refreshTime();
        void refreshTime(uint64_t now);
        bool isValid(uint64_t time);
    private:
        uint64_t m_time;
        std::unordered_map<std::string, boost::any> m_data;
        std::mutex m_mutex;
        
    };

    class SessionDataMan : public std::enable_shared_from_this<SessionDataMan>{
    public:
        using ptr = std::shared_ptr<SessionDataMan>;
        SessionDataMan(IOScheduler::ptr scheduler, uint64_t timertime);
        bool hasSessionData(const std::string& key);
        SessionData::ptr getSessionData(const std::string& key);
        void delSessionData(const std::string& key);
        void setSessionData(const std::string& key, std::vector<std::pair<std::string, boost::any>>& data);
        void setSessionData(const std::string& key, std::string key1, boost::any any);
        void addSessionData(const std::string& key);
        void timer();
    private:
        std::shared_mutex m_rwmutex;
        std::map<std::string, SessionData::ptr> m_sessionDataMap;
        //std::set<std::pair<std::string, SessionData::ptr>> m_list;
        //IOScheduler::ptr m_scheduler;
        uint64_t m_ttime;
    };
}
}

#endif