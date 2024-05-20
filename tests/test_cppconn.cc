
#include "../ttw/db/mysqlcppconn.h"
#include "../ttw/log.h"
#include "../ttw/json/jsonelement.h"
//static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
int main(){
    auto pool = ttw::db::ConnectionPool::GetConnectionPool();
    auto p = pool->getConnection();
    
    std::string id = "4";
    auto ret = p->query("select * from user where id = ?", id);
    ttw::json::JsonElement test;
    if(ret){

        while(ret->next()){
            test["id"] = ret->getInt("id");
            test["user_name"] = ret->getString("user_name");
            test["createtime"] = ret->getString("createtime");
        }
        delete ret;
    }
    LOG_DEBUG(logger_) << test.toString();
    LOG_DEBUG(logger_) << "false";
    pool->stop();
    return 0;
}
