#ifndef _TTW_LOG_H
#define _TTW_LOG_H

#include<memory>
#include<string>
#include<sstream>
#include<fstream>
#include<list>
#include<vector>
#include<map>
#include<iostream>
#include<stdarg.h>
#include<time.h>
#include<stdint.h>
#include<algorithm>
#include <functional>
#include<mutex>

#include "instantiation.h"



#define LOG_STREAM(logger, level)\
            if(logger->getLevel() <= level )\
                ttw::ContainerLogEvent(ttw::LogEvent::ptr(new ttw::LogEvent(logger, level, __FILE__, __LINE__, \
                                time(0), "name", 1))).getEevenSStream()


#define LOG_DEBUG(logger) LOG_STREAM(logger, ttw::LogLevel::DEBUG)

#define LOG_INFO(logger) LOG_STREAM(logger, ttw::LogLevel::INFO)

#define LOG_WARN(logger) LOG_STREAM(logger, ttw::LogLevel::WARN)

#define LOG_ERROR(logger) LOG_STREAM(logger, ttw::LogLevel::ERROR)

#define LOG_FATAL(logger) LOG_STREAM(logger, ttw::LogLevel::FATAL)





namespace ttw {

    

    class Logger;
    class LogLevel;
    class LogEvent;
    class LogAppender;

    class LogLevel{
        public:
        using ptr = std::shared_ptr<LogLevel>;
    
        enum Level
        {
            UNKNOW = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL
        };

        static const char* ToString(LogLevel::Level level);
        static LogLevel::Level FormString(std::string str);
    };


    class Layout{
    public:
        using ptr = std::shared_ptr<Layout>;
        Layout(const std::string& pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);
        std::ostream& format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);

        class Format {
        public:
            using ptr = std::shared_ptr<Format>;
            
            virtual ~Format() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) = 0;
        };
        bool isError() const { return m_error; }
        void explain();
    private:
        bool m_error = false;
        std::string m_pattern;
        std::vector<Format::ptr> m_formats;
        std::mutex m_tex;
    };


    class Logger : public std::enable_shared_from_this<Logger>{
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string name = "root");

        void log(LogLevel::Level level, std::shared_ptr<LogEvent> event);

        void debug(std::shared_ptr<LogEvent> event);

        void info(std::shared_ptr<LogEvent> event);

        void warn(std::shared_ptr<LogEvent> event);

        void error(std::shared_ptr<LogEvent> event);

        void fatal(std::shared_ptr<LogEvent> event);
        
        const std::string getName() { return m_name; }
        LogLevel::Level getLevel() { return m_level; }
        Layout::ptr getLayout() { return m_layout; }
        

        void setLevel(LogLevel::Level level) {; m_level = level; }

        void setLayout(Layout::ptr layout);
        void addAppender(std::shared_ptr<LogAppender> appender);
        void delAppender(std::shared_ptr<LogAppender> appender);
        void clearAppender();
    private:
        const std::string m_name;
        LogLevel::Level m_level;
        std::list<std::shared_ptr<LogAppender>> m_appenders; 
        Layout::ptr m_layout;
        std::mutex m_mutex;
    };

    class LogEvent{
    public:
        using ptr = std::shared_ptr<LogEvent>;
        LogEvent(Logger::ptr logger, LogLevel::Level level, const std::string file, uint32_t line,
                    time_t time, const std::string threadName, uint32_t threadId);

        LogLevel::Level getLevel() {return m_level;}

        Logger::ptr getLogger() { return m_logger;}
        
        time_t getTime(){ return m_time; }
        
        uint32_t getThreadId() { return m_threadId;}
        
        const std::string getFileName() { return m_fileName;}
        
        uint32_t getLine() { return m_line;}
        
        const std::string getThreadName() { return m_threadName;}

        std::stringstream& getStream() {return m_sStream;}

        std::string getMeesmage() { return m_sStream.str(); }


        void format(const char* fmt, ...);
        void format(const char *fmt, va_list al); 
    private:
        Logger::ptr m_logger;
        LogLevel::Level m_level;
        const std::string m_fileName;
        uint32_t m_line;
        time_t m_time;
        const std::string m_threadName;
        uint32_t m_threadId;
        std::stringstream m_sStream;
    };


    

    class LogAppender{
    friend class Logger;
    public:

        using ptr = std::shared_ptr<LogAppender> ;

        virtual ~LogAppender() {}

        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        LogLevel::Level getLevel() { return m_level; }
        Layout::ptr getLayout() { return m_layout;}
        bool isHasLayout() {return m_hasLayout; }

        void setLayout(Layout::ptr layout);

    protected:
        LogLevel::Level m_level = LogLevel::Level::DEBUG;
        Layout::ptr m_layout = nullptr;
        bool m_hasLayout = false;
        std::mutex m_mutex;

    };

    class ConsoleLogAppender :public LogAppender {
    public:
        using ptr = std::shared_ptr<ConsoleLogAppender>;
        ConsoleLogAppender(const std::string& str = ""){}
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    private:

    };

    class FileLogAppender : public LogAppender {
    public:
        using ptr = std::shared_ptr<FileLogAppender>;
        FileLogAppender(const std::string& filePath);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen();
    private:
        std::string m_filePath;
        std::ofstream m_fileStream;
    };

    class ContainerLogEvent{
    public:
        ContainerLogEvent(LogEvent::ptr event);
        ~ContainerLogEvent();
        std::stringstream& getEevenSStream();

    private:
        std::mutex m_mutex;
        LogEvent::ptr m_event;
    };

    class LogStream{
    public:
        LogStream(LogEvent::ptr);
        LogEvent::ptr getEevent() { return m_event; }
    private:
        LogEvent::ptr m_event;
    };

    class LogManage{
    public:
        LogManage();
        Logger::ptr getLogger(std::string name = "root");
        void addLogger(std::string name, Logger::ptr v);
        void delLogger(std::string name);
        static LogManage* GetLogManage();
    private:
        std::map<std::string, Logger::ptr> m_logMap;
        
    };
    static std::map<std::string, std::function<LogAppender::ptr(const std::string &str)>> LogAppenders_ = {
        #define XX(str, C)\
                    {#str, [](const std::string& fmt) {return LogAppender::ptr( new C(fmt) ); } }

                    XX(ConsoleLogAppender, ConsoleLogAppender),
                    XX(FileLogAppender, FileLogAppender)
        #undef XX
    };

}

//static auto logger_ = ttw::Instantiation<ttw::LogManage>::GetInstantion()->getLogger("root");

#endif