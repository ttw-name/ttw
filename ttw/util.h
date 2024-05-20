#ifndef __TTW__UTIL_H
#define __TTW__UTIL_H

#include <bits/types.h>
#include <sys/time.h>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <string>
#include <vector>


namespace ttw {
    uint64_t GetCurrentMs();
    uint64_t GetCurrentUs();
    int addFdFlag(int fd, int flag);
    int delFdflag(int fd, int flag);
    int setNonBlock(int fd,bool value = true);
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");
    void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
    std::string Time2Str(time_t ts, const std::string& format);
    
    static std::string demangle(const char* str) {
        size_t size = 0;
        int status = 0;
        std::string rt;
        rt.resize(256);
        if(1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
            char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
            if(v) {
                std::string result(v);
                free(v);
                return result;
                }
        }
        if(1 == sscanf(str, "%255s", &rt[0])) {
            return rt;
        }
        return str;
    }

}

#endif