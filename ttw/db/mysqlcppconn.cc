

#include "mysqlcppconn.h"


//static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw {
namespace db{

    static ConnectionPool::ptr s_connectionpool = nullptr;

    Connection::Connection(sql::Connection* conn, int max):m_conn(conn), m_max(max), m_count(0){
        refreshTime();
    }

    Connection::~Connection(){
        if(m_conn){
            while(m_head){
                m_tail = m_head;
                m_head = m_head->next;
                delete m_tail;
                m_tail = nullptr;
            }
            delete m_conn;
        }else{
           if(m_head || m_tail){
            exit(1);
           }
        }
    }

    Connection::Node::Node(sql::Connection* conn, const std::string& str){

        m_statement = conn->prepareStatement(str);
        m_key = str;
        next = nullptr;
        pre = nullptr;
    }

    Connection::Node::~Node(){
        if(m_statement){
            delete m_statement;
        }else{
            exit(1);
        }
    }

    void Connection::moveToHead(Node* node){

        if(!node){
            return;
        }


        if(node == m_head){
            return;
        } else if(node == m_tail){
            node->pre->next = nullptr;
            m_tail = node->pre;
            node->pre = nullptr;
            m_head->pre = node;
            node->next = m_head;
            m_head = node;
        }else{
            node->pre->next = node->next;
            node->next->pre = node->pre;
            node->next = m_head;
            m_head->pre = node;
            node->pre = nullptr;
            node->next = m_head;
            m_head = node;
        }

    }

    Connection::Node* Connection::addToHead(sql::Connection* conn, const std::string& str){
        Node* node = new Node(conn, str);
        if(m_count < m_max){
            addToHead(node);
        }else{
            //getNodeTial();
            delNodeTail();
            addToHead(node);
        }
        return node;
    }

    void Connection::addToHead(Node* node){

        if(node){

            //m_map[node->m_key] = node;
            if(m_count == 0){
                m_head = node;
                m_tail = node;
                
            }else{
                node->next = m_head;
                m_head->pre = node;
                node->pre = nullptr;
                m_head = node; 
            }
            m_map[node->m_key] = node;
            ++m_count;
        }
    }

    void Connection::delNodeTail(){
        m_map.erase(m_tail->m_key);
        auto temp = m_tail->pre;
        m_tail->pre->next = nullptr;
        delete m_tail;
        m_tail = temp;
        --m_count;
    }

    sql::PreparedStatement* Connection::getPreparedStatement(const std::string& sql){
        auto it = m_map.find(sql);
        if(it != m_map.end()){
            moveToHead(it->second);
            m_pstatement = it->second->m_statement;
        } else{
            auto t = new Node(m_conn, sql);
            addToHead(t);
            m_pstatement = t->m_statement;
        }
        
        return m_pstatement;
    }

    void Connection::refreshTime(){
        m_time = GetCurrentMs();
    }

    uint64_t Connection::getIdleTime(){
        return GetCurrentMs() - m_time;
    }

    void ConnectionPool::CreatConnctionPool(){
       s_connectionpool.reset(new ConnectionPool("tcp://127.0.0.1:3306","root", "123ttw", "ttw",
            20, 100, 5, 30 * 1000 * 60, 2000
       ));
    }

    ConnectionPool* ConnectionPool::GetConnectionPool(){
        if(s_connectionpool){
            return s_connectionpool.get();
        }
        CreatConnctionPool();
        return s_connectionpool.get();
    }

    ConnectionPool::ConnectionPool(const std::string& server, const std::string& username, const std::string& userpasswd,
                       const std::string& dbname, int min, int max, int pstaMax, uint64_t maxIdleTime, uint64_t connTime):
                       m_server(server),
                       m_username(username),
                       m_userpasswd(userpasswd),
                       m_db(dbname),
                       m_min(min),
                       m_max(max),
                       m_pstaMax(pstaMax),
                       m_maxidleTime(maxIdleTime),
                       m_connTime(connTime) {

                        m_diver = get_driver_instance();
                        for(auto i = 0; i < m_min; ++i){
                            addConnection();
                        }
                        t1 = std::thread(std::bind(&ConnectionPool::scanerConnection, this));
                        t2 = std::thread(std::bind(&ConnectionPool::produceConnection, this));



    }

    ConnectionPool::~ConnectionPool(){

    }

    void ConnectionPool::addConnection(){
        sql::Connection* conn;
        try{
            conn = m_diver->connect(m_server, m_username, m_userpasswd);
        }catch(const sql::SQLException& e){
            LOG_ERROR(logger_) << "Could not connect to server. Error message: " << e.what();
            return;
        }

        conn->setSchema(m_db);
        Connection* connp = new Connection(conn, m_pstaMax);
        m_list.push_back(connp);
        ++m_count;
    }

    void ConnectionPool::produceConnection(){
        while(!m_stop){
            std::unique_lock<std::mutex> lock(m_mutex);
            while((int)m_list.size() >= m_min){
                m_condition.wait(lock);
                if(m_stop){
                    return;
                }
            }
            if(m_count < m_max){
                addConnection();
            }
            m_condition.notify_all();
        }
    }


    void ConnectionPool::scanerConnection(){
        while(!m_stop){
            //std::this_thread::sleep_for(std::chrono::milliseconds(m_maxidleTime));
            std::unique_lock<std::mutex> lock(m_mutex);
            m_sancond.wait_for(lock, std::chrono::milliseconds(m_maxidleTime));
            while(m_count > m_min){
                auto conn = m_list.front();
                if(conn->getIdleTime() >= m_maxidleTime){
                    m_list.pop_front();
                    --m_count;
                    delete conn;
                }else{
                    break;
                }
            }
        }
    }

    Connection::ptr ConnectionPool::getConnection(){
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_list.empty()){
            if(std::cv_status::timeout == m_condition.wait_for(lock, std::chrono::microseconds(m_connTime))){
                if(m_list.empty()){
                    continue;
                }
            }
        }
        Connection::ptr sp(m_list.front(), [&](Connection* ptr){
            std::unique_lock<std::mutex> lock(m_mutex);
            ptr->refreshTime();
            m_list.push_back(ptr);
        });

        m_list.pop_front();
        m_condition.notify_all();
        return sp;
    }

    bool ConnectionPool::isStop(){
        //std::unique_lock<std::mutex> lock(m_mutex);
        return m_stop;
    }
    void ConnectionPool::stop(){
        
            
        if(m_stop){
            return;
        }
        m_stop = true;
        m_condition.notify_all();
        
        m_sancond.notify_all();
        t1.join();
        t2.join();
    }
    void ConnectionPool::start(){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(!m_stop){
            return;
        }
        m_stop = false;
        t1 = std::thread(std::bind(&ConnectionPool::scanerConnection, this));
        t2 = std::thread(std::bind(&ConnectionPool::produceConnection, this));
    }

}
}