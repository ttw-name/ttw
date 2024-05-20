#include<jsoncpp/json/json.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include<string>
#include <fstream>


int main(){
    std::ifstream in;
    in.open("/home/ttw/code/ttw/tests/testjson.json", std::ios::in | std::ios::binary);

    Json::Value root;
    Json::Value::Members members;
    Json::Reader reader;
    if(!in.is_open()){
        std::cout << "open file failure" << std::endl;;
        if (in.fail()) {
            std::cerr << "文件打开失败的原因： " << in.rdstate() << std::endl;
        return 0;}
    }
    bool is = reader.parse(in, root);
    std::cout << "tttt" << std::endl;
    if(!is)
        return 0;
    members = root.getMemberNames();
    Json::Value c;
    for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it){
        std::cout << *it << std::endl;
        c = root[*it];
        //std::string d = boost::lexical_cast<std::string>(c["name"]);
        std::cout << c.type() << std::endl;
        
        
    }
    auto cc = root["logs"];
    auto dd = root["system"];
    std::cout << "--------------------" << std::endl;
    std::cout << cc.type() << std::endl;
    Json::Value::Members members1 = cc.getMemberNames();
    std::cout << "--------------------" << std::endl;
    for(Json::Value::Members::iterator it = members1.begin(); it != members1.end(); ++it){
        std::cout << *it << std::endl;
        auto d = cc[*it];
        //std::string d = boost::lexical_cast<std::string>(c["name"]);
        std::cout << d.type() << std::endl;
    }
    std::cout << "--------------------" << std::endl;
    std::cout << dd.asString()<<std::endl;
    return 0;
}