

#include "daemon.h"
//#include "log.h"



namespace ttw
{
namespace
{
    class ExitCaller{
    public:

        ExitCaller(std::function<void()> cb){
            m_cb = cb;
        }

        ~ExitCaller(){ m_cb(); }

    private:
        std::function<void()> m_cb;
    };
}
    int Daemon::daemon_start(const std::string& pidfile){
        int pid = getPidFile(pidfile);
        if(pid > 0){
            if(kill(pid, 0) == 0 || errno == EPERM){
                return -1;
            }
        }

        if(getppid() == 1){
            return -1;
        }

        pid = fork();
        if(pid < 0){
            return -1;
        }

        if(pid > 0){
            exit(0);
        }

        setsid();
        int ret = writePidFile(pidfile);
        if(ret != 0){
            return ret;
        }

        int fd = open("/dev/null", 0);
        if(fd >= 0){
            close(0);
            dup2(fd, 0);
            dup2(fd, 1);
            close(fd);

            std::string pf = pidfile;
            static ExitCaller del( [pf]{ unlink(pf.c_str()); });
            return 0;
        }
        return -1;

    }

    // int Daemon::daemon_start(int argc, char** argv, std::function<int(int, char**)> cb){

    //     daemon(1, 0);
    //     while(true){
    //         pid_t pid = fork();
    //         if(pid == 0){
    //             return cb(argc, argv);
    //         } else if(pid < 0){
    //             return -1;
    //         }else{
    //             int status = 0;
    //             waitpid(pid, &status, 0);
    //             if(status){
    //                 if(status == 9){
    //                     //LOG_INFO(logger_) << "kill -9";
    //                     break;
    //                 }else{
    //                     //LOG_ERROR(logger_) << "pid = " << pid << " status = " << status;
    //                 }
    //             } else {
    //                 //LOG_INFO(logger_) << "finished pid = " << pid;
    //                 break;
    //             }
    //             sleep(5);
    //         }
    //     }
    // }

    int Daemon::daemon_restart(const std::string& pidfile){
        int pid = getPidFile(pidfile);
        if(pid > 0){
            if(kill(pid, 0) == 0){
                int ret = daemon_stop(pidfile);
                if(ret < 0){
                    return ret;
                }
            }else if(errno == EPERM){
                return -1;
            }
        }
        return daemon_start(pidfile);
    }
    int Daemon::daemon_stop(const std::string& pidfile){
        int pid = getPidFile(pidfile);
    if (pid <= 0) {
        return -1;
    }

    int ret = kill(pid, SIGQUIT);
    if (ret < 0) {
        return ret;
    }

    for (int i = 0; i < 300; i++) {
        usleep(10*1000);
        ret = kill(pid, SIGQUIT);
        if (ret != 0) {
            unlink(pidfile.c_str());
            return 0;
        }
    }

    return -1;
    }


    int Daemon::writePidFile(const std::string& pidfile){
        char str[64];
        int fp = open(pidfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if(fp < 0 || lockf(fp, F_TLOCK, 0) < 0){
            return -1;
        }

        ExitCaller call([fp]{ close(fp); });
        sprintf(str, "%d\n", getpid());
        size_t len = strlen(str);
        size_t ret = write(fp, str, len);
        if(ret != len){
            return -1;
        }
        return 0;
    }
    int Daemon::getPidFile(const std::string& pidfile){
        char buffer[64];
        char* p;

        int fp = open(pidfile.c_str(), O_RDONLY, 0);
        if(fp < 0){
            return fp;
        }

        size_t ret = read(fp, buffer, 64);
        close(fp);
        if(ret <= 0){
            return -1;
        }
        buffer[63] = '\0';
        p = strchr(buffer, '\n');
        if(p != NULL){
            *p = '\0';
        }
        return atoi(buffer);
    }

    void Daemon::daemonProcess(const std::string& cmd, const std::string& pidfile){
        int ret = 0;
        if(cmd.empty() || cmd.compare("start") == 0){
            ret = daemon_start(pidfile);
        }else if(cmd.compare("stop") == 0){
            ret = daemon_stop(pidfile);
            if(ret == 0){
                exit(0);
            }
        }else if(cmd.compare("restart") == 0){
            ret = daemon_restart(pidfile);
        }else{
            fprintf(stderr, "ERROR: bad daemon command. exit\n");
            ret = -1;
        }
        if(ret){
            exit(0);
        }
    }
    void Daemon::changeTo(const char* argv[]){
        int pid = getpid();
        int r = fork();
        if (r < 0) {
            fprintf(stderr, "fork error %d %s", errno, strerror(errno));
        } else if (r > 0) { //parent;
            return;
        } else { //child
            //wait parent to exit
            while(kill(pid, 0) == 0) {
                usleep(10*1000);
            }
            if (errno != ESRCH) {
                const char* msg = "kill error\n";
                ssize_t w1 = write(2, msg, strlen(msg));
                (void)w1;
                _exit(1);
            }
            execvp(argv[0], (char* const*)argv);
        }
    }
    
namespace {
    std::map<int, std::function<void()>> handlers;
    void signal_handler(int signal){
        handlers[signal]();
    }   
}

void Signal::signal(int signal, const std::function<void()>& cb){
    handlers[signal] = cb;
    ::signal(signal, signal_handler);
}




} // namespace ttw


