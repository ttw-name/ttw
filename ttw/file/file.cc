
#include <algorithm>

#include <sys/types.h>

#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "ttw/log.h"
#include "file.h"

static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw{
namespace file{
    
    bool isImgType(const std::string& filename){

        return getFileTypeEnum(filename) != FileType::unknown;

    }
    std::string getFileTypeString(const std::string& filename){
        std::string tion;
        for(size_t i = filename.size() - 1; i >= 0; --i){
            if(filename[i] == '.'){
                if(i + 1 < filename.size()){
                    tion = filename.substr(i + 1);
                    break;
                }
            }
        }
        return tion;
    }
    FileType getFileTypeEnum(const std::string& filename){
        std::string tion = getFileTypeString(filename);
        if(tion.empty()){
            return FileType::unknown;
        }

        return StringtoFileType(tion);
    }

    FileType StringtoFileType(const std::string& type){

        if(type.empty()){
            return FileType::unknown;
        }
        std::string typea = type;
        std::transform(typea.begin(), typea.end(), typea.begin(), [](unsigned char c) { return std::tolower(c);});
        if(!typea.compare("ioc")){
            return FileType::ioc;
        }
        if(!typea.compare("png")){
            return FileType::png;
        }
        if(!typea.compare("jpeg")){
            return FileType::jpeg;
        }
        if(!typea.compare("jpg")){
            return FileType::jpg;
        }
        if(!typea.compare("gif")){
            return FileType::gif;
        }
        if(!typea.compare("json")){
            return FileType::json;
        }

        return FileType::unknown;
    }

    bool createDir(const std::string& dirname, int mode){
        if(mkdir(dirname.c_str(), mode) == -1){
            LOG_ERROR(logger_) << "Dircreate fail strerron = " << strerror(errno);
            return false;
        }
        return true;
    }
    void deleteDir(const std::string& dirname){
        
        if(dirname.empty()){
            return;
        }

        DIR* dir = opendir(dirname.c_str());
        struct dirent* entry;
        char path[1024];

        if(dir){
            while((entry = readdir(dir)) != NULL){
                if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                    snprintf(path, sizeof(path), "%s/%s", dirname.c_str(), entry->d_name);
                    if(entry->d_type == DT_DIR){
                        deleteDir(path);
                    }else{
                        unlink(path);
                    }
                }
            }
            closedir(dir);
        }

        if(rmdir(dirname.c_str()) != 0){
            LOG_ERROR(logger_) << "delteDir : " << dirname << " fail!! " << strerror(errno);
        }

        
    }

    bool createFile(const std::string& filename){
        return false;
    }
    bool deleteFile(const std::string& filename){
        if(access(filename.c_str(), F_OK) != 0){
            return true;
        }
        if(unlink(filename.c_str()) != 0){
            LOG_ERROR(logger_) << "file : " << filename << " deleteFile fail!! " << strerror(errno);
            return false;
        }
        return true;
    }

    std::vector<std::string> getFileList(const std::string& path, const std::string& prefix){

        std::vector<std::string> ret;

        DIR* dir  = opendir(path.c_str());
        struct dirent* entry;

        if(!dir){
            ret.clear();
            return ret;
        }

        while((entry = readdir(dir)) != NULL){
            if(entry->d_type != DT_DIR){
                std::string t(entry->d_name);
                t = prefix + t;
                ret.push_back(t);
            }
        }
        closedir(dir);
        return ret;

    }

    bool deleteFilePrefix(const std::string& path, const std::string& prefix){
        DIR* dir  = opendir(path.c_str());
        struct dirent* entry;

        if(!dir){
            LOG_DEBUG(logger_) << "open dir fail! " << strerror(errno);
            return false;
        }
        std::string filename;
        while((entry = readdir(dir)) != NULL){
            if(entry->d_type == DT_REG){
                filename = entry->d_name;
                if(filename.find(prefix) == 0){
                    if(!deleteFile(path + "/" + filename)){
                        closedir(dir);
                        return false;
                    }
                }
                
            }
        }
        return true;

    }

    void getFileAllList(const std::string& path, std::vector<std::string>& vec, 
                        const std::vector<std::string>& losedirs, const std::string& prefix){
        DIR* dir  = opendir(path.c_str());
        struct dirent* entry;

        if(!dir){
            return;
        }

        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                if(entry->d_type == DT_DIR){
                    std::string t(entry->d_name);
                    bool flag = true;
                    for(auto it : losedirs){
                        if(t.compare(it) == 0){
                           flag = false;
                        }
                    }
                    if(flag){
                         getFileAllList(path + "/" + t, vec, losedirs, prefix + t + "/");
                    }
                        
                }else{
                    std::string t(entry->d_name);
                    t = prefix + t;
                    vec.push_back(t);
                }
            }
        }
        closedir(dir);
    }

    std::vector<std::string> getChildernList(const std::string& path){
        std::vector<std::string> ret;

        DIR* dir  = opendir(path.c_str());
        struct dirent* entry;

        if(!dir){
            ret.clear();
            return ret;
        }

        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                if(entry->d_type == DT_DIR){
                    std::string t(entry->d_name);
                    ret.push_back(t);
                }
            }
        }
        closedir(dir);
        return ret;
    }
    

    bool renameFile(const std::string& filename, const std::string& targetname){
        if(access(filename.c_str(), F_OK) != 0){
            return false;
        }

        if(rename(filename.c_str(), targetname.c_str()) != 0){
            LOG_WARN(logger_) << "renameFile fail : " << filename << " --> " << targetname << " erron = " << strerror(errno);
            return false;
        }

        return true;

    }
    
    long int getFileSize(const std::string& filename){
        if(access(filename.c_str(), F_OK) != 0){
            return -1;
        }

        struct stat file_stat;
        if(stat(filename.c_str(), &file_stat) == 0){
            return file_stat.st_size;
        }
        return -1;

    }
    int getFileMode(const std::string& filename){
        if(access(filename.c_str(), F_OK) != 0){
            return -1;
        }

        struct stat file_stat;
        if(stat(filename.c_str(), &file_stat) == 0){
            return file_stat.st_mode;
        }
        return -1;
    }
    char* getFileContext(const std::string& filename){
        return NULL;
    }

    File::File(const std::string& path, bool free) : m_path(path), m_isvalid(false) {
        m_data = nullptr;
        std::ifstream file(m_path, std::ios::binary | std::ios::ate);
        if(!file.is_open()){
            LOG_WARN(logger_) << "filename : " << m_path << " opne fail!" << " strerron " << strerror(errno);
            claer();
            return;
        }
        m_size = file.tellg();
        file.seekg(0, std::ios::beg);
        m_data = new char[m_size];

        if(!file.read(m_data, m_size)){
            LOG_WARN(logger_) << "filename : " << m_path << " read fail!";
            claer();
            return;
        }
        file.close();

        struct stat file_stat;
        if(stat(m_path.c_str(), &file_stat) != 0){
            LOG_WARN(logger_) << "filename : " << m_path << " get permission fail!";
            claer();
            return;
        }
        m_mode = file_stat.st_mode;
        m_type = getFileTypeEnum(m_path);
        if(m_type != FileType::unknown){
            m_isvalid = true;
        }else{
            //claer();
        }
        //m_isvalid = true;
    }

    File::File(char* data, const std::string& path, long int size, int mode, bool free):
     m_path(path),
     m_data(data),
     m_size(size),
     m_mode(mode),
     m_isvalid(false),
     m_free(free)
    {
        m_type = getFileTypeEnum(path);
        if(m_type != FileType::unknown){
            m_isvalid = true;
        }else{
            claer();
        }
    }

    File::File() :m_path(""), m_data(nullptr), m_type(FileType::unknown), m_size(-1), m_mode(0755), m_isvalid(false), m_free(false){

    }

    File::~File(){
        LOG_DEBUG(logger_) << "~File : " << m_path;
        claer();
    }

    void File::claer(){
        if(m_free && m_data != nullptr){
            delete[] m_data;
            m_data = nullptr;
            m_size = -1;
        }
        m_isvalid = false;
    }

    std::string File::getPath() const {
        return m_path;
    }

    void File::setPath(const std::string& path){
        m_path = path;
    }

    long int File::getFileSize() const {
        return m_size;
    }

    void File::setFileSize(long int size){
        m_size = size;
    }

    int File::getFileMode() const {
        return m_mode;
    }

    void File::setFileMode(int mode){
        m_mode = mode;
    }

    char* File::getFileData() const {
        return m_data;
    }

    void File::setFileData(char* data){
        m_data = data;
    }

    bool File::isType(const std::string& type) const {

        return StringtoFileType(type) == m_type;
    }
    bool File::isType(const FileType& type) const {
        return type == m_type;
    }
    bool File::save() {
        if(m_isvalid){
            std::ofstream file(m_path, std::ios::binary);
            if(!file){
                LOG_ERROR(logger_) << "filename : " << m_path << " save fail!";
                return false;
            }
            file.write(m_data, m_size);
            if(file.good()){
                 //LOG_INFO(logger_) << "save";
            }else{
                file.close();
                return false;
            }
            file.close();
            chmod(m_path.c_str(), m_mode);
           
            return true;
        }
        return false;
    }

    bool File::isValid() const {
        return m_isvalid;
    }

    void File::refresh(){
        if(!m_path.empty()){
            m_type = getFileTypeEnum(m_path);
        }
        if(m_data != nullptr && m_size != -1 && (!m_path.empty()) && m_type != FileType::unknown){
            m_isvalid = true;
        }else{
            m_isvalid = false;
        }
    }
}
}