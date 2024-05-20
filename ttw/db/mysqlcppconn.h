

#ifndef __TTW__DB__MYSQLCPPCONN_H
#define __TTW__DB__MYSQLCPPCONN_H


#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <thread>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include <list>
#include <atomic>


#include "../util.h"
#include "../log.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw{
namespace db{

    class Connection : public std::enable_shared_from_this<Connection>{
    public:
        using ptr = std::shared_ptr<Connection>;

        Connection(sql::Connection* conn, int max = 5);
        ~Connection();

        template<typename... Args>
        bool update(const std::string& sql, Args... args){
            try{
                auto p = getPreparedStatement(sql);
                bindArgs(1, args...);
                p->executeUpdate();
                return true;
            }catch(sql::SQLException& e){
                LOG_WARN(logger_) << "sql update fail : " << sql << ". " << e.what();
            }
            return false;
        }

        bool update(const std::string& sql){
            auto p = getPreparedStatement(sql);
            try{
                p->executeUpdate();
                return true;
            }catch(sql::SQLException& e){
                LOG_WARN(logger_) << "sql update fail : " << sql << ". " << e.what();
            }
            return false;
        }

        template<typename... Args>
        sql::ResultSet* query(const std::string& sql, Args... args)  {
            sql::ResultSet* res = nullptr;
            try{
                getPreparedStatement(sql);
                bindArgs(1, args...);
                res = m_pstatement->executeQuery();
            } catch(sql::SQLException& e){
                LOG_WARN(logger_) << "sql query fail : " << sql << ". " << e.what();
            }
            return res;
        }

        sql::ResultSet* query(const std::string& sql){
            sql::ResultSet* res = nullptr;
            try{
                getPreparedStatement(sql);
                res = m_pstatement->executeQuery();
            }catch(sql::SQLException& e){
                LOG_WARN(logger_) << "sql query fail : " << sql << ". " << e.what();
            }
            return  res;
        }

        sql::PreparedStatement* getPreparedStatement(const std::string& sql);

        void refreshTime();
        uint64_t getIdleTime();

    private:
        
         template<typename T, typename... Args>
         void bindArgs(int index, T t, Args... args){
            //sql::PreparedStatement* pstmt  = (sql::PreparedStatement*)pstmtaddr;

            if constexpr (std::is_same_v<T, char>){
                m_pstatement->setString(index, t);
            }else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>){
                m_pstatement->setString(index, t);
            }else if constexpr (std::is_same_v<T, int>){
                m_pstatement->setInt(index, t);
            }else {
                //pstmt->setNull(index, NULL);
                exit(1);
            }
            bindArgs(++index, args...);
         }

         template<typename T>
         void bindArgs(int index, T t){
            //sql::Connection* conn = (sql::Connection*)(*connaddr);
            //sql::PreparedStatement* pstmt  = (sql::PreparedStatement*)pstmtaddr;

             if constexpr (std::is_same_v<T, char>){
                  m_pstatement->setString(index, t);
             }else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>){
                 m_pstatement->setString(index, t);
             }else if constexpr (std::is_same_v<T, int>){
                 m_pstatement->setInt(index, t);
             }else {
                 //pstmt->setNull(index, NULL);
                exit(1);
             }

         }

        struct Node{
            Node(sql::Connection* conn, const std::string& str);

            //std::string* rest(const std::string& str);

            Node* next;

            Node* pre;

            sql::PreparedStatement* m_statement; 
            std::string m_key;
            ~Node();        
        };

        void moveToHead(Node* node);
        Node* addToHead(sql::Connection* conn, const std::string& str);
        void addToHead(Node* node);
        void delNodeTail();
        Node* getNodeTial(bool del = true);


    private:
        sql::Connection* m_conn;
        sql::PreparedStatement* m_pstatement;
        int m_max;
        int m_count;
        Node* m_head;
        Node* m_tail;
        std::unordered_map<std::string, Node*> m_map;
        uint64_t m_time;
        //std::list<Node*> m_list;
    };


    class ConnectionPool : public std::enable_shared_from_this<ConnectionPool>{
    public:
        using ptr = std::shared_ptr<ConnectionPool>;

        static void CreatConnctionPool();
        static ConnectionPool* GetConnectionPool();

        ConnectionPool(const std::string& server, const std::string& uername, const std::string& userpasswd,
                       const std::string& dbname, int min, int max, int pstaMax, uint64_t maxIdleTime, uint64_t connTime);
        ~ConnectionPool();

        Connection::ptr getConnection();
        
        bool isStop();
        void stop();
        void start();

        void addConnection();
        void produceConnection();
        void scanerConnection();

    private:
        std::string m_server;
        std::string m_username;
        std::string m_userpasswd;
        std::string m_db;
        std::list<Connection*> m_list;
        sql::Driver* m_diver;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::condition_variable m_sancond;
        int m_min;
        int m_max;
        int m_pstaMax;
        uint64_t m_maxidleTime;
        uint64_t m_connTime;
        std::atomic<int> m_count{0};
        std::atomic<bool> m_stop{false};
        //std::vector<std::thread> m_threads;
        std::thread t1;
        std::thread t2;
    };
}
}


#endif

