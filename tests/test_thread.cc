
#include "ttw/thread.h"
#include "ttw/log.h"
#include "ttw/config.h"
#include<iostream>
#include<chrono>

std::atomic<int> t = {0};
void sum(){
    LOG_DEBUG(logger_);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    t.fetch_add(1,std::memory_order_consume);
}

int main(){
    auto tp = new ttw::ThreadPool("main", 100);
    std::cout << "----------------" << std::endl;
    for(int i = 0; i < 1000; ++i){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        tp->add(sum);
        
    }
    while(tp->getTaskQuec()){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        LOG_DEBUG(logger_) << "works: " << tp->getWorkThreads();
        LOG_DEBUG(logger_) << "ides: " << tp->getIdeThreads();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    LOG_DEBUG(logger_) << "works: " << tp->getWorkThreads();
    LOG_DEBUG(logger_) << "ides: " << tp->getIdeThreads();
    delete tp;
    LOG_DEBUG(logger_) << "t = " << t;

}