
#include <iostream>

#include "db/mysqlconn.h"

int main(){
    
    auto t = ttw::db::ConnectionPool::GetConnctionPool();
    auto conn = t->getConnection();
    if(conn == nullptr){
        std::cout << "rron!";
    }
    //t->stop();
    int a;
    std::cin >> a;
}

