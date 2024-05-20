#include "filecache.h"

namespace ttw{
namespace file{
    FileCache::FileCache(File::ptr file): m_isValid(false){
        //m_date.reset(std::string)
        if(file && file->getFileData() && file->getFileSize() != -1){
            m_data = std::make_shared<std::string>(file->getFileData(), file->getFileSize());
            m_path = file->getPath();
            m_isValid = true;
        }else{
            m_data = nullptr;
            m_path.clear();
            m_isValid = false;
        }

    }


    FileCache::~FileCache(){

    }


    std::shared_ptr<std::string> FileCache::getFileData(){
        if(m_isValid){
            return m_data;
        }else{
            return nullptr;
        }
        //return m_data;
    }


    void FileCache::setFileData(const std::string& data){
        m_data = std::make_shared<std::string>(data);
    }

    std::string& FileCache::getPath() {
        return m_path;
    }

    LRUCache::Node::Node(FileCache::ptr filecache):next(nullptr), pre(nullptr), m_fileCache(filecache) {

    }

    LRUCache::LRUCache(int max):m_head(nullptr),m_tail(nullptr), m_max(max), m_conut(0){

    }

    

    LRUCache::~LRUCache(){
        clear();
    }

    FileCache::ptr LRUCache::getFileCache(const std::string& path){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_cacheNode.find(path);
            if(it != m_cacheNode.end()){
                auto node = it->second;
                moveToHead(node);
                return node->m_fileCache;
            }
        }
        if(m_conut < m_max){
            return addToHead(path);
        }

        
        return addToHead(path, true);

    }
    void LRUCache::delCacheNode(){
        
        if(m_tail){
            auto path = m_tail->m_fileCache->getPath();
            m_cacheNode.erase(path);
        }
        if(m_tail && m_tail == m_head){
            delete m_tail;
            m_tail = nullptr;
            m_head = nullptr;
        }else{
            auto temp = m_tail->pre;
            delete m_tail;
            m_tail = temp;
            m_tail->next = nullptr;
        }
        --m_conut;
    }
    void LRUCache::moveToHead(Node* node){
        //std::unique_lock<std::mutex> lock(m_mutex);
        if(!node){
            return;
        }

        if(node == m_head){
            return;
        }

        if(node != m_tail){
            node->pre->next = node->next;
            node->next->pre = node->pre;
            node->pre = nullptr;
            node->next = m_head;
            m_head->pre = node;
            m_head = node;
        }else{
            m_tail = m_tail->pre;
            m_tail->next = nullptr;
            node->next = m_head;
            m_head->pre = node;
            m_head = node;
            m_head->pre = nullptr;
        }
    }

    FileCache::ptr LRUCache::addToHead(Node* node){
        if(m_head){
            node->next = m_head;
            m_head->pre = node;
            m_head = node;
        }else{
            m_head = node;
            m_tail = node;
        }
        ++m_conut;
        return node->m_fileCache;
    }

    void LRUCache::moveToHead(File::ptr file){

    }
    FileCache::ptr LRUCache::addToHead(File::ptr file, bool del){
        FileCache::ptr t(new FileCache(file));
        if(!t->isValid()){
            return nullptr;
        }
        auto node = new Node(t);

        std::unique_lock<std::mutex> lock(m_mutex);
        if(del){
            delCacheNode();
        }
        m_cacheNode[t->getPath()] = node;
        return addToHead(node);
    }

    FileCache::ptr LRUCache::addToHead(const std::string& path, bool del){
        std::shared_ptr<File> t(new File(path, true));
        return addToHead(t, del);
    }

    void LRUCache::clear(){
        std::unique_lock<std::mutex> lock(m_mutex);
        for(auto it = m_cacheNode.begin(); it != m_cacheNode.end(); ++it){
            delete it->second;
        }
        m_cacheNode.clear();
        m_tail = nullptr;
        m_head = nullptr;
        m_conut = 0;
    }

    int LRUCache::getMax() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_max;
    }

    void LRUCache::setMax(int size){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_max = size;
    }

    bool LRUCache::isExists(const std::string& path){
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_cacheNode.find(path);
        if(it != m_cacheNode.end()){
            return true;
        }else{
            return false;
        }
    }

    FileCacheMan::FileCacheMan(const std::string& name, int conunt):m_name(name) {
        m_tempCache.reset(new LRUCache(conunt));
    }

    FileCacheMan::~FileCacheMan(){
        m_perpenCache.clear();
    }

    std::shared_ptr<std::string> FileCacheMan::getFileCacheData(const std::string& path){
        {
            std::shared_lock<std::shared_mutex> r_lock(m_rwmutex);
            auto it = m_perpenCache.find(path);
            if(it != m_perpenCache.end()){
                return it->second->getFileData();
            }
        }
        auto t = m_tempCache->getFileCache(path);
        if(t){
            return t->getFileData();
        }else{
            return nullptr;
        }
        
    }

    void FileCacheMan::addFileCacheData(File::ptr val, bool temp){
        if(temp){

        }else{

        }
    }

    void FileCacheMan::addFileCacheData(FileCache::ptr val){
        
    }

    bool FileCacheMan::addFileCacheData(const std::string& path, bool rejion){

        auto it = m_perpenCache.find(path);
        if(it != m_perpenCache.end()){
            if(!rejion){
                return true;
            }
        }

        File::ptr f(new File(path, true));
        FileCache::ptr fc(new FileCache(f));
        if(!fc->isValid()){
            return false;
        }
        //addFileCacheData(fc);
        m_perpenCache[path] = fc;
        return true;

    }

    void FileCacheMan::delFileCacheDate(const std::string& path){
        auto it = m_perpenCache.find(path);
        if(it != m_perpenCache.end()){
            m_perpenCache.erase(it);
        }
    }

    void FileCacheMan::dleFileCacheDate(){

    }

    bool FileCacheMan::isExists(const std::string& path){
        auto it = m_perpenCache.find(path);
        if(it != m_perpenCache.end()){
            return true;
        }

        return m_tempCache->isExists(path);
    }


}
}