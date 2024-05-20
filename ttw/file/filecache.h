#ifndef __TTW__FILE_CACHE_H
#define __TTW__FILE_CACHE_H

#include "file.h"
#include <memory>
#include <map>
//#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include <mutex>



namespace ttw{
namespace file{

    
    class FileCache : public std::enable_shared_from_this<FileCache>{
    public:
        using ptr = std::shared_ptr<FileCache>;
        FileCache(File::ptr file);

        ~FileCache();

        std::shared_ptr<std::string> getFileData();

        void setFileData(const std::string& date);

        std::string& getPath();
        void setPath(const std::string& path);
        bool isValid() const { return m_isValid; }

    private:
        std::string m_path;
        bool m_isValid;
        std::shared_ptr<std::string> m_data;
    };

    class LRUCache : public std::enable_shared_from_this<LRUCache>{

        struct Node{
            Node(FileCache::ptr filecache);

            Node* next;

            Node* pre;

            FileCache::ptr m_fileCache;
        };
    public:
        using ptr = std::shared_ptr<LRUCache>;

        LRUCache(int max);

        ~LRUCache();

        FileCache::ptr getFileCache(const std::string& path);

        void delCacheNode();

        void moveToHead(Node* node);

        FileCache::ptr addToHead(Node* node);

        void moveToHead(File::ptr file);

        FileCache::ptr addToHead(File::ptr file, bool del);
        FileCache::ptr addToHead(const std::string& path, bool del = false);


        void clear();
        int getMax();
        void setMax(int size);

        bool isExists(const std::string& path);

    private:

        Node* m_head;

        Node* m_tail;

        int m_max;

        int m_conut;

        std::map<std::string, Node*> m_cacheNode;

        std::mutex m_mutex;
    };

    class FileCacheMan : public std::enable_shared_from_this<FileCacheMan>{
    public:
        using ptr = std::shared_ptr<FileCacheMan>;

        FileCacheMan(const std::string& name, int conunt = 64);

        ~FileCacheMan();

        std::shared_ptr<std::string> getFileCacheData(const std::string& ptah);

        void addFileCacheData(File::ptr val, bool temp = true);

        void addFileCacheData(FileCache::ptr val);

        bool addFileCacheData(const std::string& path, bool rejion = false);

        void delFileCacheDate(const std::string& path);

        void dleFileCacheDate();

        bool isExists(const std::string& path);
    private:
        std::string m_name;
        std::map<std::string, FileCache::ptr> m_perpenCache;
        LRUCache::ptr m_tempCache;
        std::shared_mutex m_rwmutex;
    };
}
}
#endif