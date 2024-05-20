#include<functional>
#include "log.h"
namespace ttw {
    
    
    const char* LogLevel::ToString(LogLevel::Level level) {
        switch(level){
    #define XX(name) \
        case LogLevel::name:\
                return #name;\
                break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL)
    #undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }

    static LogManage* t_logManage = nullptr;

    LogLevel::Level LogLevel::FormString(std::string str){
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    #define XX(level, v) \
    if(str == #v) { \
        return LogLevel::level; \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);
    return LogLevel::UNKNOW;
#undef XX
    }

    Layout::Layout(const std::string& pattern):m_pattern(pattern){
        explain();
    }

    std::string Layout::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
        
        std::unique_lock<std::mutex> t(m_tex);
        std::stringstream ss;
        for(auto &i : m_formats){
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    std::ostream& Layout::format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
        std::unique_lock<std::mutex> t(m_tex);
        for(auto &i : m_formats){
            i->format(os, logger, level, event);
        }
        return os;
    }

    

    class MessageFormat : public Layout::Format {
    public:
        MessageFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getMeesmage();

        }
    };

    class LevelFormat : public Layout::Format {
    public:
        LevelFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << LogLevel::ToString(level);
        }
    };

    class ThreadIdFormat : public Layout::Format {
    public:
        ThreadIdFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getThreadId();
        }
    };

    class NameFormat : public Layout::Format {
    public:
        NameFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getLogger()->getName();
        }
    };

    class ThreadNameFormat : public Layout::Format {
    public:
        ThreadNameFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getThreadName();
        }
    };

    class DateTimeFormat : public Layout::Format {
    public:
        DateTimeFormat(const std::string& str = "%Y-%m-%d %H:%M:%S") : m_format(str) {
            if(m_format.empty()){
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FileNameFormat : public Layout::Format {
    public:
        FileNameFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getFileName();
        }
    };

    class LineFormat : public Layout::Format {
    public:
        LineFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << event->getLine();
        }
    };
    
    class LineBreakFormat : public Layout::Format {
    public:
        LineBreakFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os <<   std::endl;  
        }
    };

    class StringFormat : public Layout::Format {
    public:
        StringFormat(const std::string& str) :m_string(str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << m_string;
        }
    private:
        std::string m_string;
        
    };

    class TabFormat : public Layout::Format {
    public:
        TabFormat(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override {
            os << '\t';    
        }
    };
    
    //%d{} %d 
    void Layout::explain(){
        std::vector<std::tuple<std::string, std::string, bool>> vec;
        std::string nstr;

        for(size_t i = 0; i < m_pattern.size(); ++i){

            if(m_pattern[i] != '%'){
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if((i + 1) < m_pattern.size()){
                if(m_pattern[i + 1] == '%'){
                    nstr.append(1, '%');
                    i = i + 1;
                    continue;
                }
            }
            size_t n = i + 1;
            bool flag = false;
            std::string fmt, str;
            if(n < m_pattern.size()){
                fmt.append(1, m_pattern[n++]);
            }
            if(n < m_pattern.size()){
                if(m_pattern[n] == '{'){
                    n++;
                    while(n < m_pattern.size()){
                        if(m_pattern[n] == '}'){
                            flag = true;
                            break;
                        }
                        str.append(1, m_pattern[n]);
                        ++n;
                    }
                }
            }
            
            i = flag ? n : n - 1;
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr, "", false));
                nstr.clear();
            }

            vec.push_back(std::make_tuple(fmt, str, true));
        }

        if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr, "", false));
                nstr.clear();
            }

        static std::map<std::string, std::function<Format::ptr( const std::string &str)>> formatsMap = {
        #define XX(str, C)\
                {#str, [](const std::string& fmt) {return Format::ptr( new C(fmt) ); } }

                XX(m, MessageFormat),
                XX(p, LevelFormat),
                XX(c, NameFormat),
                XX(n, LineBreakFormat),
                XX(d, DateTimeFormat),
                XX(f, FileNameFormat),
                XX(l, LineFormat),
                XX(N, ThreadNameFormat),
                XX(T, TabFormat),
        #undef XX
        };

        for(auto& i : vec){
            if(std::get<2>(i) == false){
                m_formats.push_back(Format::ptr(new StringFormat(std::get<0>(i))));
            }else{
                
                auto it = formatsMap.find(std::get<0>(i));
                if(it == formatsMap.end()){
                    m_error = true;
                    return;
                }
                
                m_formats.push_back(it->second(std::get<1>(i)));
            }
        }
        
    }


    Logger::Logger(const std::string name)
    :m_name(name),
    m_level(LogLevel::DEBUG)
    {
    
        m_layout.reset(new Layout("%d{%Y-%m-%d %H:%M:%S}%T[%p]%T[%c]%T%f:%l%T%m%n"));
        
    }
    
    void Logger::log(LogLevel::Level level, LogEvent::ptr event){
        

        if(level >= m_level){
            
            auto self = shared_from_this();
            // if(m_appenders.empty()){
            //     self->addAppender(std::shared_ptr<LogAppender>(new ConsoleLogAppender));
            // }
            std::unique_lock<std::mutex> lock(m_mutex);
            for(auto& i : m_appenders){
                i->log(self, level, event);
            }
        }

    }

    void Logger::setLayout(Layout::ptr layout){
        m_layout = layout;
        //std::unique_lock<std::mutex> lock(m_mutex);
        for(auto& i : m_appenders){
                i->m_layout = layout;
        }
    }

    void Logger::addAppender(LogAppender::ptr appender){
        //std::unique_lock<std::mutex> lock(m_mutex);
        if(!appender->getLayout()){
            appender->setLayout(m_layout);
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender){
        //std::unique_lock<std::mutex> lock(m_mutex);
        for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it){
            if(*it == appender){
                m_appenders.erase(it);
                return;
            }
        }
    }
    void Logger::clearAppender(){
        m_appenders.clear();
    }

    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::DEBUG, event);
     }

    void Logger::info(LogEvent::ptr event){
        log(LogLevel::INFO, event);
     }
     
    void Logger::warn(LogEvent::ptr event){
        log(LogLevel::WARN, event);
     }

    void Logger::error(LogEvent::ptr event){
        log(LogLevel::ERROR, event);
     }

    void Logger::fatal(LogEvent::ptr event){
        log(LogLevel::FATAL, event);
     }

    LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level, const std::string file, uint32_t line,
                    time_t time, const std::string threadName, uint32_t threadId) :
                    m_logger(logger),
                    m_level(level),
                    m_fileName(file),
                    m_line(line),
                    m_time(time),
                    m_threadName(threadName),
                    m_threadId(threadId)
                    {}
    void LogEvent::format(const char* fmt, ...){
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char* fmt, va_list al){
            char* buf = nullptr;
            int l = vasprintf(&buf, fmt, al);
            if(l != -1){
                m_sStream << std::string(buf, l);
                free(buf);
            }
    }
    //void LogEvent::format(const char* fmt, ...);
    void LogAppender::setLayout(Layout::ptr v){
        m_layout = v;
    }

    FileLogAppender::FileLogAppender(const std::string& filePath): m_filePath(filePath){
        if(!reopen()){
            throw std::runtime_error("open file failure!");
        }
    }

    bool FileLogAppender::reopen(){
        if(m_fileStream){
            m_fileStream.close();
        }
        m_fileStream.open(m_filePath, std::ios::app);
        return m_fileStream.is_open();
    }

    void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
        
        if(level >= m_level){
            //std::unique_lock<std::mutex> lock(m_mutex);
             if(!m_layout->format(m_fileStream, logger, level, event)){
                throw std::runtime_error("file stream error");
             }
             m_fileStream << std::endl;
        }
    }

    void ConsoleLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
        
        if(level >= m_level){
            //std::unique_lock<std::mutex> lock(m_mutex);
             if(!m_layout->format(std::cout, logger, level, event)){
                throw std::runtime_error("string stream error");
             }
        }
    }

    ContainerLogEvent::ContainerLogEvent(LogEvent::ptr event): m_event(event){

    }
    ContainerLogEvent::~ContainerLogEvent(){
        //std::unique_lock<std::mutex> lock(m_mutex);
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }
    std::stringstream& ContainerLogEvent::getEevenSStream(){
        //std::unique_lock<std::mutex> lock(m_mutex);
        return m_event->getStream();
    }

    LogStream::LogStream(LogEvent::ptr event) :m_event(event){}

    LogManage::LogManage(){

        }
    Logger::ptr LogManage::getLogger(std::string name){
            auto t = m_logMap.find(name);
            if(t == m_logMap.end()){
                if(name == "root"){
                    m_logMap[name] = std::shared_ptr<Logger>(new Logger); 
                    return m_logMap[name];   
                }
                return nullptr;
            }
            

            return m_logMap[name];
        }
    void LogManage::addLogger(std::string name, Logger::ptr v){
        if(v != nullptr)
            m_logMap[name] = v;
    }
    void LogManage::delLogger(std::string name){
        auto it = m_logMap.find(name);
        if(it != m_logMap.end()){
            m_logMap.erase(name);
        }
    }

    LogManage* LogManage::GetLogManage(){
        if(t_logManage){
            return t_logManage;
        }

        t_logManage = new LogManage;
        return t_logManage;

    }
}