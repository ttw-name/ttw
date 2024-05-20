#include"ttw/instantiation.h"
#include"ttw/log.h"

//auto t = ttw::Instantiation<ttw::Logger>::GetInstantion();

int main(){
    //
    auto logger = ttw::Instantiation<ttw::LogManage>::GetInstantion()->getLogger("root");
    LOG_DEBUG(logger) << "cccc";
    LOG_INFO(logger) << "INFO";
    return 0;
    
}




