

#include "config.h"
#include "log.h"
#include "ttwassert.h"
#include <string>
#include <fstream>
#include <sstream>



namespace ttw{

    static Config::ptr s_config = nullptr;

    Config::Config(const std::string& path):m_path(path), m_iserron(false){
        loadFile();
        if(m_iserron){

        }
    }

    void Config::loadFile(){
        std::string str;
        std::stringstream ss;
        std::ifstream file(m_path);
        if(file){
            ss << file.rdbuf();
            str = ss.str();
            //LOG_DEBUG(logger_) <<  "\n" << str;
           // ASSERT(true);
        }else{
            m_iserron = true;
            //ASSERT();
            return;
        }

        json::JsonElement v;
        json::Reader reader(str);
        if(reader.parser(v)){
            if(v.isObject()){
                loadMap(v);
            }
        }else{
            m_iserron = true;
            ASSERT(false);
        }
        
        std::string log = getConfigValue<std::string>("log", "");
        if(!log.compare("true")){
            configLog();
        }

    }

    void Config::loadMap(const json::JsonElement& v, const std::string& key) {
        
        if(m_iserron){
            return;
        }

        if(v.isObject()){
           for(auto& it : v.getJsonObject()){
            std::string newkey;
            if(key.empty()){
                newkey = it.first;
            }else{
                newkey = key + "." + it.first;
            }
                loadMap(it.second, newkey);
           }
        }else if(v.isArray()) {
            m_iserron = true;
            m_map.clear();
            return;
        } else if(v.isString()){
            std::string str = v.toString();
            ASSERT(str.length() >= 2);
            
                
            auto n = str.substr(1, str.length() - 2);
            m_map[key] = n;
        }else{
            std::string n = v.toString();
            m_map[key] = n;
        }
    }

    bool Config::isErron() const {
        return m_iserron;
    }

    Config* Config::GetConfigInstance(){
        if(s_config){
            return s_config.get();
        }

        s_config.reset(new Config("default.json"));
        return s_config.get();
    }

    void Config::SetConfigInstance(const std::string& path){
        s_config.reset(new Config(path));
    }

    void Config::configLog(){
        auto logger = LogManage::GetLogManage()->getLogger("root");
        std::string format = getConfigValue<std::string>("logformat", "%d{%Y-%m-%d %H:%M:%S}%T[%p]%T[%c]%T%f:%l%T%m%n");
        logger->setLayout(Layout::ptr(new Layout(format)));
        std::string level = getConfigValue<std::string>("loglevle", "info");
        auto loglevel = LogLevel::FormString(level);
        logger->setLevel(loglevel);
        // std::string logname = getConfigValue<std::string>("logname", "root");
        // logger->setName(logname);
        std::string logfile = getConfigValue<std::string>("logfile", "");
        if(logfile.empty()){
            logger->addAppender(FileLogAppender::ptr(new FileLogAppender(logfile)));
        }else{
            logger->addAppender(ConsoleLogAppender::ptr(new ConsoleLogAppender));
        }
    }
    
} // namespace ttw
