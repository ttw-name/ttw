#include<ttw/log.h>
#include<ttw/config.h>


static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
int main(){

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Current path: " << cwd << std::endl;
    } else {
        std::cerr << "Error getting the current working directory" << std::endl;
    }

    // Json::Reader::parse()
    auto conf = ttw::Config::GetConfigInstance();
    auto test = conf->getConfigValue<int>("coroutinestactsize", 123);
    LOG_DEBUG(logger_) << test;
    LOG_DEBUG(logger_) << conf->isErron();
    return 0;
}


