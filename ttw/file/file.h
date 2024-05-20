
#ifndef __TTW__FILE_H
#define __TTW__FILE_H

#include <string>
#include <memory>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <shared_mutex>

namespace ttw {
namespace file{
    
    enum class FileType{
        ioc = 0,
        jpg,
        jpeg,
        png,
        gif,
        bmp,
        raw,
        heif,
        json,
        unknown
    };

    std::string FileTypetoString(const FileType& type);
    FileType StringtoFileType(const std::string& type);

    bool isImgType(const std::string& filename);
    std::string getFileTypeString(const std::string& filename);
    FileType getFileTypeEnum(const std::string& filename);

    bool createDir(const std::string& dirname, int mode = 0777);
    void deleteDir(const std::string& dirname);

    bool createFile(const std::string& filename);
    bool deleteFile(const std::string& filename);

    std::vector<std::string> getFileList(const std::string& path, const std::string& prefix = "");
    void getFileAllList(const std::string& path, std::vector<std::string>& vec, 
                        const std::vector<std::string>& losedirs, const std::string& prefix = "");

    bool deleteFilePrefix(const std::string& path, const std::string& prefix);
    
    std::vector<std::string> getChildernList(const std::string& path);
    

    bool renameFile(const std::string& filename);
    
    long int getFileSize(const std::string& filename);
    int getFileMode(const std::string& filename);
    char* getFileContext(const std::string& filename);

    class File{
    public:
        using ptr = std::shared_ptr<File>;

        File(const std::string& path, bool free = false);
        File(char* date, const std::string& path, long int size, int mode = 0755, bool free = false);
        File();
        ~File();

        std::string getPath() const;

        void setPath(const std::string& path);


        long int getFileSize() const;
        void setFileSize(long int size);


        bool isValid() const;

        int getFileMode() const;
        void setFileMode(int mode);

        char* getFileData() const;
        void setFileData(char* data);

        bool isType(const std::string& type) const;
        bool isType(const FileType& type) const;
        bool save();
        void claer();
        void refresh();

    private:
        std::string m_path;
        char* m_data;
        FileType m_type;
        long int m_size;
        int m_mode;
        bool m_isvalid;
        bool m_free;
    };


}
}


#endif