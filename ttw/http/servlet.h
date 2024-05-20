

#pragma once


#include "http.h"
#include "http_session.h"

#include <memory>
#include <string>
#include <shared_mutex>
#include <cxxabi.h>

namespace ttw {
namespace http{


class Servlet {
public:

    using ptr = std::shared_ptr<Servlet>;

   
    Servlet(const std::string& name)
        :m_name(name) {}

    virtual ~Servlet() {}

    virtual int32_t handle(ttw::http::HttpRequest::ptr request
                   , ttw::http::HttpResponse::ptr response
                   , ttw::http::HttpSession::ptr session) = 0;
                   

    const std::string& getName() const { return m_name;}
protected:
    std::string m_name;
};


class FunctionServlet : public Servlet {
public:
    
    using ptr = std::shared_ptr<FunctionServlet>;
    
    using callback = std::function<int32_t (ttw::http::HttpRequest::ptr request
                   , ttw::http::HttpResponse::ptr response
                   , ttw::http::HttpSession::ptr session)>;


    FunctionServlet(callback cb);
    virtual int32_t handle(ttw::http::HttpRequest::ptr request
                   , ttw::http::HttpResponse::ptr response
                   , ttw::http::HttpSession::ptr session) override;
private:
    
    callback m_cb;
};

class IServletCreator {
public:
    using ptr = std::shared_ptr<IServletCreator>;
    virtual ~IServletCreator() {}
    virtual Servlet::ptr get() const = 0;
    virtual std::string getName() const = 0;
};

class HoldServletCreator : public IServletCreator {
public:
    using ptr = std::shared_ptr<HoldServletCreator>;
    HoldServletCreator(Servlet::ptr slt)
        :m_servlet(slt) {
    }

    Servlet::ptr get() const override {
        return m_servlet;
    }

    std::string getName() const override {
        return m_servlet->getName();
    }
private:
    Servlet::ptr m_servlet;
};

template<class T>
class ServletCreator : public IServletCreator {
public:
    using ptr = std::shared_ptr<ServletCreator>;

    ServletCreator() {
    }

    Servlet::ptr get() const override {
        return Servlet::ptr(new T);
    }

    std::string getName() const override {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }
};


class ServletDispatch : public Servlet {
public:
    
    using ptr =  std::shared_ptr<ServletDispatch>;
   
   

    ServletDispatch();
    virtual int32_t handle(ttw::http::HttpRequest::ptr request
                   , ttw::http::HttpResponse::ptr response
                   , ttw::http::HttpSession::ptr session) override;

 
    void addServlet(const std::string& uri, Servlet::ptr slt);

    void addServlet(const std::string& uri, FunctionServlet::callback cb);


    void addGlobServlet(const std::string& uri, Servlet::ptr slt);

    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
    void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

    template<class T>
    void addServletCreator(const std::string& uri) {
        addServletCreator(uri, std::make_shared<ServletCreator<T> >());
    }

    template<class T>
    void addGlobServletCreator(const std::string& uri) {
        addGlobServletCreator(uri, std::make_shared<ServletCreator<T> >());
    }


    void delServlet(const std::string& uri);

   
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getDefault() const { return m_default;}


    void setDefault(Servlet::ptr v) { m_default = v;}



    Servlet::ptr getServlet(const std::string& uri);


    Servlet::ptr getGlobServlet(const std::string& uri);

    Servlet::ptr getMatchedServlet(const std::string& uri);

    void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
    void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
private:

    std::shared_mutex m_mutex;


    std::unordered_map<std::string, IServletCreator::ptr> m_datas;

    std::vector<std::pair<std::string, IServletCreator::ptr> > m_globs;

    Servlet::ptr m_default;
};


class NotFoundServlet : public Servlet {
public:
    
    using ptr = std::shared_ptr<NotFoundServlet>;
    
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(ttw::http::HttpRequest::ptr request
                   , ttw::http::HttpResponse::ptr response
                   , ttw::http::HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};

}
}