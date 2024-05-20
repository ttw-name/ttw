#include "log.h"
#include "config.h"
#include "ioscheduler.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <thread>
auto schedu = new ttw::IOScheduler("root", false, 2);
int sock = 0;
void test_sch(){
    
    schedu->start();
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    inet_pton(AF_INET, "39.156.66.10", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))){

    }else if(errno == EINPROGRESS){
        LOG_INFO(logger_) << "add event errno=" << errno << " " << strerror(errno);
        schedu->addEvent(sock, ttw::READ, [](){
            LOG_INFO(logger_) << "read callback";
           
        });

        schedu->addEvent(sock, ttw::WRITE, [](){
            LOG_INFO(logger_) << "write callback";
            schedu->cancelEvent(sock, ttw::READ);
        });


    }else {
        LOG_INFO(logger_) << "else" << errno << " " << strerror(errno);
    }
    LOG_DEBUG(logger_) << "sock: " << sock;
    
}

void test_timer(){
    LOG_DEBUG(logger_) << "timer ....";
}

int main(){
    
    test_sch();
    //std::shared_ptr<ttw::Timer> t(new ttw::Timer(200, test_timer, true));
    schedu->addTimer(1000, &test_timer, true);
    std::shared_ptr<int> t(0);
    std::weak_ptr<int> wt(t);
    schedu->addConditionTimer(2000, &test_timer, wt, false);
    
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    //schedu->cancelTimer(t);
    schedu->join();
    return 0;
}