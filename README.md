## 这是一个简单的C/C++网络库

## 基于epoll与线程、协程实现的多线程多协程的Reactor模式的服务器模型；

## 系统与依赖

系统：Ubuntu 20.04 LTS

编译器：g++

C\+\+版本：C\+\+17

第三方库：boost、MySQL、Connector/C++

依赖：pthread、mysqlcpponn



## 项目结构

*   tests			一些功能测试例子
*   ttw             库的源代码

    *   json      内部的Json解析库
    *   db         支持MySQL的数据库连接池
    *   file        对应着一些文件操作
    *   http       实现http1.1

## 基于本网络库实现的网站

### [http://47.245.85.138/](http://47.245.85.138/)

## 使用例子

```cpp
#include <ttw/ttw.h>
using namespace ttw;

int main(){
    http::HttpServer::ptr server(new http::HttpServer(2, "ttw"));
    server->bind("127.0.0.1", 8000);
    auto sd = server->getServletDispatch();
    
    sd->addServlet("/index", [](ttw::http::HttpRequest::ptr req
                            ,http::HttpResponse::ptr rsp
                            ,http::HttpSession::ptr session){
                
                rep->setBody("Hello World!");    
                return 0;
                });
    server->start();
    //while(1); 
    server->jion();
    return 0;
}

```

### 如果需要使用到配置，在开始获取一下Config实例

```cpp
#include <ttw/ttw.h>
using namespace ttw;
int main(){
     //Config::GetConfigInstance(); ///需要获取一下，配置才会生效
     auto config = Config::SetConfigInstance("yourfile"); ///如果自定义路径，可以进行Set
	 int threads = config->getConfigValue("threads", 1);  //后面的是默认值，如果配置文件中没有，则返回默认值
     http::HttpServer::ptr server(new http::HttpServer(threads, "ttw"));
     std::string host =  config->getConfigValue<std::string>("host", "127.0.0.1");
     int port = config->getConfigValue("port", 8000);
    server->bind(host, port);
    //....
}

```

### 使用日志

```cpp
	//...
	static logger = LogManage::GetLogManage()->getLogger("root");
	sd->addServlet("/index", [&](ttw::http::HttpRequest::ptr req
                                ,http::HttpResponse::ptr rsp
                                ,http::HttpSession::ptr session){
                    LOG_DUG(logger) << "your logs";
                    rep->setBody("Hello world!");    
                    return 0;
                    });
	//...
```

### 如果需要使用到数据库

```cpp
    //...
    auto conpool = db::ConnectionPool::GetConnectionPool();
    sd->addServlet("/index", [&](ttw::http::HttpRequest::ptr req
                                ,http::HttpResponse::ptr rsp
                                ,http::HttpSession::ptr session){
                    std::string sql = "select s1, s2, s3 from ttw where s1 = ? and s2 = ?";
                    int s1 = 100;
                    std::string s2 = "ttw"
                    auto com = conpool->getConnection();
                    sql::PreparedStatement* result = com->query(sql, s1, s2); //支持可变参数
                     /*
                        使用ttw的Json解析器
                    */
                    json::JsonElement ret;
                    if(!result){ //判断数据查询是否出错
                        ret["success"] = false;
                        rsp->setBody(ret.toString());
                        return 0;
                    }
                   
                   json::JsonElement arr(json::JsonElement::JsonType::ARRY);
                   while(result->next()){
                       json::JsonElement v;
                       v["s1"] = result->getString(1);
                       v["s2"] = result->getString(2);
                       //...
                       arr.append(v);
                    }
                    ret["arr"] = arr;
                    rep->setBody(ret.toString());    
                    return 0;
                    });


```

### 使用文件缓存

```cpp
	//...
	file::FileCacheMan::ptr cache(new file::FileCacheMan("ttw", 128)); //第一个参数缓存管理的名字，第二个缓存最大数
	sd->addGlobServlet("/image/*", [&](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session){

                    auto path = req->getPath();
                    rsp->addHeaderKey("Content-Type", "image/png");
                    auto t = cache->getFileCacheData("yourdir" + path);
                    if(t){
                        rsp->setBody(*t);
                    }else{
                        rsp->setStatus(http::HttpStatus::NOT_FOUND);
                    }
                    return 0;
	//...
```

### 使用cookie与session

```cpp
	//... 
	http::HttpServer::ptr server(new http::HttpServer(2, "ttw"));
	http::SessionDataMan::ptr sessiondata(new http::SessionDataMan(server->getScheduler(), 30 * 60 * 1000)); //
	//...
	sd->addServlet("/api/user/login", [&](ttw::http::HttpRequest::ptr req
                ,ttw::http::HttpResponse::ptr rsp
                ,ttw::http::HttpSession::ptr session){
                    
                    if(req->getMethod() ==  http::HttpMethod::POST){
                        rsp->addHeaderKey("Content-Type", "application/json");
                        std::string getkey = req->getHeaderValue("cookie");
                        auto data = sessiondata->getSessionData(getkey);
                        if(data){
                            rsp->setBody("{\"success\" : true}");          
                            return 0;
                        }
                        std::string name = req->getQuery("username");
                        std::string pwd = req->getQuery("password"); 
                        if(name.empty() || pwd.empty()){
                            rsp->setBody("{\"success\" : false}");
                            return 0;
                        }
                        
                        /*
                            通过认证
                        */
                        std::vector<std::pair<std::string, boost::any>> vec;
                        vec.emplace_back("id", boost::any(id));
                        //...
                        std::string session_val = "yoursessionval"
                        sessiondata->addSessionData(session_val);
                        sessiondata->setSessionData(session_val, vec);
                        rsp->setCookie("yoursessionkey", session_val, time(0) + (30 * 60 * 1000) - 8 * 60 * 60, "yourpath", "yourdomain");
						return 0;
					}
```

## 参考

该服务的http模块与协程调度模块的思路主要参考与sylar框架，编写细节与结构有一定差异。
