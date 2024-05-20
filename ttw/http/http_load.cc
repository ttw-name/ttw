

#include "http_load.h"


namespace ttw{
namespace http{
    void LoadFile(ServletDispatch::ptr dispatch, file::FileCacheMan::ptr cache, const std::string& path, const std::vector<std::string>& losedirs){
        
        std::vector<std::string> vec;
        file::getFileAllList(path, vec, losedirs, "/");

        for(auto it : vec){
            cache->addFileCacheData(path + it);
        }

        for(auto it : vec){
            std::string key = path + it;
            dispatch->addServlet(it, [key, cache](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session) {
            if(req->getMethod() == HttpMethod::GET){
                rsp->setBody(*cache->getFileCacheData(key));
            }
            
            return 0;
    });
        }
    }

}
}