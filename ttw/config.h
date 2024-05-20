#ifndef __TTW_CONFIG_H
#define __TTW_CONFIG_H


#include <string>
#include <map>
#include <exception>
#include <list>
#include <fstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <stack>

#include "log.h"
#include "json/jsonelement.h"
#include "json/reader.h"







namespace ttw {

    class Config{
    public:
        using ptr = std::shared_ptr<Config>;
        Config(const std::string& path);
        void loadFile();
        bool isErron() const;
        template<class T>
        T getConfigValue(const std::string& key, T def){
            auto it = m_map.find(key);
            if(it == m_map.end()){
                return def;
            }

            std::string val = m_map[key];
            try{
                T ret = boost::lexical_cast<T>(val);
                return ret;
            } catch(...){
                return  def;
            }
        } 
        void loadMap(const json::JsonElement& v, const std::string& key = "");
        static Config* GetConfigInstance();
        static void SetConfigInstance(const std::string& path);
        void configLog();

    private:
        std::string m_path;
        bool m_iserron;
        std::map<std::string, std::string> m_map;
    };
}


#endif