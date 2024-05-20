// #include "corscheduler.h"
// #include "log.h"
// #include "config.h"


// void cor(){
//     LOG_INFO(logger_) << "cor begin!!";
//     ttw::Coroutine::ToSUSPENDED();
//     LOG_INFO(logger_) << "cor end!!";
//     ttw::Coroutine::ToSUSPENDED();

//     //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
// }
// auto g = new ttw::CorScheduler("root", 5);
// void cor1(){
//     static int s = 50;
//     LOG_INFO(logger_) << " test in cors : " << s;
//     //sleep(1);
//     LOG_INFO(logger_) << std::this_thread::get_id();
//     if(--s >= 0){
//         g->add(&cor);
//     }
    
// }

// int main(){
//     g->start();
//     g->add(&cor1);
//     // g->start();
//     // for(int i = 0; i < 100; ++i)
//     // g->add(&cor1);
//     //int a, b;
//     // while(true){
//     //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     //     LOG_DEBUG(logger_) << "----------";

//     //     g->add(&cor);
        
//     // }
//     //g->add(&cor);
//     //g->add(&cor);
//     //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//     //g->add(&cor);
//     LOG_DEBUG(logger_) << "count : " << ttw::Coroutine::GetCoroutineCount();

//     LOG_DEBUG(logger_) << "count : " << ttw::Coroutine::GetCoroutineCount();
//     //g->stop();
//     //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//     //g->join();
//     g->join();
//     g->stop();

//     int a;
//     std::cin >> a;
    
// }