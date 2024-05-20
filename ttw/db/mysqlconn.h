

#ifndef __TTW__MYSQLCONN_H
#define __TTW__MYSQLCONN_H



#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread>

#include <mysql/mysql.h>


namespace ttw {
namespace db{

    class Connection{
    public:

    using ptr = std::shared_ptr<Connection>;
        Connection();
        ~Connection();

        bool connect(const std::string& ip, 
                    const uint16_t port, 
                    const std::string& user, 
                    const std::string& password, 
                    const std::string& dbname);
        
        bool update(const std::string& sql);


        MYSQL_RES* query(const std::string& sql);


        void refreshTime();
        uint64_t getIdleTime();
    protected:



    private:
        MYSQL* m_conn;
        uint64_t m_time;
    };

    class ConnectionPool{
    public:
        using ptr = std::shared_ptr<ConnectionPool>;

        static void CreatConnctionPool();
        static ConnectionPool::ptr GetConnctionPool();

        ConnectionPool(const std::string& ip, 
                    const uint16_t port, 
                    const std::string& user, 
                    const std::string& password, 
                    const std::string& dbname, uint64_t conntime = 20 * 1000, size_t minsize = 20, size_t maxsize = 2000);
        ~ConnectionPool();

        Connection::ptr getConnection();

        void addConnection();
        void produceConnection();
        void scanerConnection();

        void stop();



    protected:


    private:
        std::string m_ip;
        uint16_t m_port;
        std::string m_user;
        std::string m_password;
        std::string m_db;
        std::atomic<bool> m_stop = {false};


        size_t m_minsize;
        size_t m_maxsize;
        uint64_t m_maxidleTime;
        uint64_t m_connTime;

        std::mutex m_mutex;
        std::condition_variable m_cond;
        std::queue<Connection*> m_connectionQueue;

        std::atomic<uint64_t> m_connectionCount = {0};
        std::vector<std::thread> m_thread;

    };


}
}


#endif