

#include "http_file.h"
#include "../log.h"
#include "../util.h"
static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw{
namespace http{


    HttpFile::HttpFile(HttpRequest::ptr req, const std::string& path, bool use_upload)
                       :m_request(req), m_filename(path), m_useupload(use_upload) {
        m_file.reset(new file::File);
        parser();
    }
    void HttpFile::init() {
        
    }

    bool HttpFile::isValid() const {
        return m_file->isValid();
    }

    bool HttpFile::save(){
        m_file->refresh();
        return m_file->save();
    }
    int HttpFile::getMode() const{
        return m_mode;
    }
    void HttpFile::setMode(int mode){
        m_mode = mode;
    }
    HttpRequest::ptr HttpFile::getRequest() const{
        return m_request;
    }
    std::string HttpFile::getFilename() const{
        return m_filename;
    }
    void HttpFile::setFilename(const std::string& filename){
        m_filename = filename;
    }
    long int HttpFile::getSize() const{
        return m_size;
    }
    file::File::ptr HttpFile::getFile() const{
        return m_file;
    }

    bool HttpFile::parser(){
        std::string boundary = m_request->getHeaderValue("Content-Type");
        if(boundary.empty()){
            return false;
        }
        if(boundary.find("multipart/form-data") == std::string::npos){
            return false;
        }
        boundary = boundary.substr(boundary.find('=') + 1);
        if(boundary.empty()){
            return false;
        }


        std::string body = m_request->getBody();

        for(size_t i = 0; i < boundary.size(); ++i){
            if(body[i+2] != boundary[i]){
                LOG_DEBUG(logger_) << "!=";
                return false;
            }
        }
        size_t pos = boundary.size() + 2 + 1 + 1;


        for(; pos < body.size(); ++pos){
            if(body[pos] == '\n'){
                break;
            }
        }
        
        if(m_useupload){
            size_t p = pos;
            --p;
            for(; p >= 0; --p){
                if(body[p] == '"'){
                    break;
                }
            }
            --p;
            for(; p >= 0; --p){
                if(body[p] == '"'){
                    break;
                }
            }
            m_filename += body.substr(p + 1, pos - (p + 2 + 1));
            LOG_DEBUG(logger_) << m_filename;
        }

        ++pos;

        for(;pos < body.size(); ++pos){
            if(body[pos] == '/'){
                break;
            }
        }

        size_t p = pos + 1;
        for(; pos < body.size(); ++pos){
            if(body[pos] == '\n'){
                break;
            }
        }
        if(!m_useupload){
            //m_filename.clear();
            
            m_filename += std::to_string(GetCurrentMs());
            m_filename += std::to_string(GetCurrentUs());
            m_filename += ".";
            m_filename += body.substr(p, pos - (p + 1));
        }

        auto pos1 = body.find(boundary);
        
        auto pos2 = body.rfind(boundary);
        
        if(pos1 == pos2){
            return false;
        }
        ++pos;
        for(;pos < body.size(); ++pos){
            if(body[pos] == '\n'){
                break;
            }
        }
        ++pos;
        m_size = pos2 - pos - 2 - 1;

        m_file->setFileData(&body[pos]);
        m_file->setFileSize(m_size);
        m_file->setPath(m_filename);

        return true;

    }
    
}
}