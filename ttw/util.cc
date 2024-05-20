#include "util.h"
#include "config.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw {
    uint64_t GetCurrentMs(){
        struct timeval now;

        int t = gettimeofday(&now, nullptr);
        
        if(t != 0){
            LOG_ERROR(logger_) << "GetCurrentMS time fail!";
            return 0;
        }


        return now.tv_sec * 1000ul + now.tv_usec / 1000;
    }

    uint64_t GetCurrentUs(){
        struct timeval now;

        int t = gettimeofday(&now, nullptr);
        
        if(t != 0){
            LOG_ERROR(logger_) << "GetCurrentUS time fail!";
            return 0;
        }


        return now.tv_sec * 1000 * 1000ul + now.tv_usec;
    }

    int addFdFlag(int fd, int flag){
        int ret = fcntl(fd, F_GETFL);
        if(ret == -1){
            LOG_ERROR(logger_) << "fcntl F_GETFD erron!!";
            return ret;
        }
        ret = fcntl(fd, F_SETFL, ret | flag);
        if(ret == -1){
            LOG_ERROR(logger_) << "fcntl F_SETFD erron!!";
        }
        return ret;
    }
    
    int delFdflag(int fd, int flag){
        int ret = fcntl(fd, F_GETFL);
        if(ret == -1){
            LOG_ERROR(logger_) << "fcntl F_GETFD erron!!";
            return ret;
        }
        ret = fcntl(fd, F_SETFL, ret & ~flag);
        if(ret == -1){
            LOG_ERROR(logger_) << "fcntl F_SETFD erron!!";
        }
        return ret;
    }

    int setNonBlock(int fd, bool value){
        if(value){
            return addFdFlag(fd, O_NONBLOCK);
        }else{
            return delFdflag(fd, O_NONBLOCK);
        }
    }




    void Backtrace(std::vector<std::string>& bt, int size, int skip) {
        void** array = (void**)malloc((sizeof(void*) * size));
        size_t s = ::backtrace(array, size);

        char** strings = backtrace_symbols(array, s);
        if(strings == NULL) {
            LOG_ERROR(logger_) << "backtrace_synbols error";
            return;
        }

        for(size_t i = skip; i < s; ++i) {
            bt.push_back(demangle(strings[i]));
        }

        free(strings);
        free(array);
    }

    std::string BacktraceToString(int size, int skip, const std::string& prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for(size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }


    std::string Time2Str(time_t ts, const std::string& format) {
        struct tm tm;
        localtime_r(&ts, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }
}