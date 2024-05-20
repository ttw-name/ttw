

#include "mysqlconn.h"
#include "log.h"
#include "config.h"
#include "util.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw {
namespace db{

    static ConnectionPool::ptr s_connectionpool = nullptr;

    void ConnectionPool::CreatConnctionPool(){
       s_connectionpool.reset(new ConnectionPool("127.0.0.1", 3306, "root", "123ttw", "ttw"));
    }

    ConnectionPool::ptr ConnectionPool::GetConnctionPool(){
        if(s_connectionpool){
            return s_connectionpool;
        }
        CreatConnctionPool();
        return s_connectionpool;

    }


    Connection::Connection(){
        m_conn = mysql_init(nullptr);
        mysql_set_character_set(m_conn, "utf8");
    }

    Connection::~Connection(){
        if(m_conn != nullptr){
            mysql_close(m_conn);
        }
    }

    bool Connection::connect(const std::string& ip, 
                    const uint16_t port, 
                    const std::string& user, 
                    const std::string& password, 
                    const std::string& dbname){

            m_conn = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
            if(m_conn == nullptr){
                LOG_ERROR(logger_) << "MySql conn fail! strron : " << mysql_errno(m_conn);
            
                return false;
            }
            LOG_DEBUG(logger_) << "connction !";
            return true;


    }

    bool Connection::update(const std::string& sql){

        if(mysql_query(m_conn, sql.c_str()) == 0){
            return true;
        }
        LOG_INFO(logger_) << "sql uptate " << sql << " uptate fail : " << mysql_errno(m_conn);
        return false;
        
    }


    MYSQL_RES* Connection::query(const std::string& sql){
        if(mysql_query(m_conn, sql.c_str()) == 0){
            return mysql_store_result(m_conn);
        }
        LOG_INFO(logger_) << "sql query " << sql << "query fail";
        return nullptr;
    }

    void Connection::refreshTime(){
        m_time = GetCurrentMs();
    }

    uint64_t Connection::getIdleTime(){
        return GetCurrentMs() - m_time;
    }

    ConnectionPool::ConnectionPool(const std::string& ip, 
                    const uint16_t port, 
                    const std::string& user, 
                    const std::string& password, 
                    const std::string& dbname, uint64_t conntime, size_t minsize, size_t maxsize):
                    m_ip(ip),
                    m_port(port),
                    m_user(user),
                    m_password(password),
                    m_db(dbname),
                    m_minsize(minsize), m_maxsize(maxsize), m_connTime(conntime)
                    {
                        LOG_DEBUG(logger_) << "connctionPool";
                        for(size_t i = 0; i < m_minsize; ++i){
                            addConnection();
                        }

                        m_thread.resize(2);
                        m_thread[0] = std::thread(std::bind(&ConnectionPool::produceConnection, this));
                        m_thread[1] = std::thread(std::bind(&ConnectionPool::scanerConnection, this));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        LOG_DEBUG(logger_) << "connctionPool end";
                    }

    ConnectionPool::~ConnectionPool(){
        while(!m_connectionQueue.empty()){
            auto p = m_connectionQueue.front();
            m_connectionQueue.pop();
            delete p;
        }
    }

    Connection::ptr ConnectionPool::getConnection(){
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_connectionQueue.empty()){
            if(std::cv_status::timeout == m_cond.wait_for(lock, std::chrono::microseconds( m_connTime))){
                if(m_connectionQueue.empty()){
                    continue;
                }
            }
        }
        Connection::ptr sp(m_connectionQueue.front(), [&](Connection* ptr){
            std::unique_lock<std::mutex> lock(m_mutex);
            ptr->refreshTime();
            m_connectionQueue.push(ptr);
        });

        m_connectionQueue.pop();
        m_cond.notify_all();
        return sp;
        
    }

    void ConnectionPool::addConnection(){
        Connection* conn = new Connection;
        conn->connect(m_ip, m_port, m_user, m_password, m_db);
        conn->refreshTime();
        m_connectionQueue.push(conn);
        ++m_connectionCount;
    }

    void ConnectionPool::produceConnection(){
        while(!m_stop){
            std::unique_lock<std::mutex> lock(m_mutex);
            while(m_connectionQueue.size() >= m_minsize && !m_stop){
                m_cond.wait(lock);
            }
            if(m_connectionCount < m_maxsize){
                addConnection();
            }
            m_cond.notify_all();
        }
    }

    void ConnectionPool::scanerConnection(){
        while(!m_stop){
            std::this_thread::sleep_for(std::chrono::milliseconds(m_maxidleTime));
            std::unique_lock<std::mutex> lock(m_mutex);
            while(m_connectionCount > m_minsize){
                auto conn = m_connectionQueue.front();
                if(conn->getIdleTime() >= m_maxidleTime){
                    m_connectionQueue.pop();
                    --m_connectionCount;
                    delete conn;
                }else{
                    break;
                }
            }
        }
    }

    void ConnectionPool::stop(){
        if(m_stop){
            return;
        }

        m_stop = true;
        for(int i = 0; i < 2; ++i){
            m_cond.notify_all();
            m_thread[i].join();
        }
        
    }
    

    

}
}