
#include "log.h"
#include "http/httpserver.h"
#include "http/http_file.h"
#include "file/file.h"
#include "file/filecache.h"
ttw::http::HttpServer::ptr server (new ttw::http::HttpServer(1, "ttw", false));
auto filecache = new ttw::file::FileCacheMan("ttw", 1);
void run(){
    
    server->bind("192.168.80.128", 8080);
    auto sd = server->getServletDispatch();
    sd->addServlet("/ttw/xx", [](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session) {
            //LOG_DEBUG(logger_) << "\n" << req->getBody();

            std::shared_ptr<ttw::http::HttpFile> t(new ttw::http::HttpFile(req, "image/"));
            
            rsp->addHeaderKey("Content-Type", "application/json");
            if(t->save()){
                rsp->setBody("{\"msg\" : true}");
            }else{
                rsp->setBody("{\"msg\" : false}");
            }

            return 0;
    });

    sd->addGlobServlet("/image/*", [](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session){

                    auto image = req->getPath();
                    //image = image.substr(image.find('/') + 1);
                    //LOG_DEBUG(logger_) << image;
                    rsp->addHeaderKey("Content-Type", "image/png");
                    //std::shared_ptr<ttw::http::HttpFile> t(new ttw::http::HttpFile(req));
                    
                    //std::shared_ptr<ttw::file::File>  t(new ttw::file::File(&image[1], true));
                    auto t = filecache->getFileCacheData(&image[1]);
                    // if(t->getFileData()){
                    //     std::string str(t->getFileData(), t->getFileSize());
                    //     rsp->setBody(str);
                    // }else{
                    //     rsp->setStatus(ttw::http::HttpStatus::NOT_FOUND);

                    // }

                    if(t){
                        //std::string str(t->getFileData(), t->getFileSize());
                        rsp->setBody(*t);
                    }else{
                        rsp->setStatus(ttw::http::HttpStatus::NOT_FOUND);

                    }
                    return 0;
                });

    sd->addGlobServlet("/ttw/*", [](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session) {
            rsp->addHeaderKey("Content-Type", "application/json");
            rsp->setBody("{\"msg\" : 30}");
            return 0;
    });


    server->start();
    int a ;
    std::cin >> a;
}

int main(){
    logger_->setLevel(ttw::LogLevel::INFO);
    //LOG_INFO(logger_) << logger_->getLevel();
    LOG_INFO(logger_) << logger_->getLevel();
    run();
    int a;
    std::cin >> a;
}

