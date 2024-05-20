#ifndef __TTW__HTTP__HTTP_FILE_H
#define __TTW__HTTP__HTTP_FILE_H

#include "file/file.h"
#include "http.h"

namespace ttw {
namespace http{

    class HttpFile : public std::shared_ptr<HttpFile>{
    public:
        using ptr = std::shared_ptr<HttpFile>;
        HttpFile(HttpRequest::ptr req, const std::string& path, bool use_upload = false);
        void init();
        bool save();
        int getMode() const;
        void setMode(int mode);
        HttpRequest::ptr getRequest() const;
        std::string getFilename() const;
        void setFilename(const std::string& filename);
        long int getSize() const;
        file::File::ptr getFile() const;
        bool isValid() const;
        bool parser();
    private:

        
        HttpRequest::ptr m_request;
        std::string m_filename;
        long int m_size;
        int m_mode;
        file::File::ptr m_file;
        bool m_useupload;
    };
}
}

#endif