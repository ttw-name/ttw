
#ifndef __TTW__DAEMON_H
#define __TTW__DAEMON_H

#include <functional>
#include <signal.h>
#include <string>
#include <utility>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>

namespace ttw {

    class Daemon{
    public:
        static int daemon_start(const std::string& pidfile);
        //static int daemon_start(int argc, char** argv, std::function<int(int, char**)> cb);
        static int daemon_restart(const std::string& pidfile);
        static int daemon_stop(const std::string& pidfile);
        static int writePidFile(const std::string& pidfile);
        static int getPidFile(const std::string& pidfile);
        static void daemonProcess(const std::string& cmd, const std::string& pidfile);
        static void changeTo(const char* argv[]);
    };

    class Signal{
    public:
        static void signal(int signal, const std::function<void()>& cb);
    };


}


#endif