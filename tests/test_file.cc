

#include "file/file.h"
#include "log.h"
#include <memory.h>



int main(){
    // auto t = new ttw::file::File("/home/ttw/code/ttw/tests/testjson.json");
    // LOG_DEBUG(logger_) << t->getFileSize();
    // LOG_DEBUG(logger_) << t->isValid();
    // LOG_DEBUG(logger_) << t->getPath();
    // LOG_DEBUG(logger_) << ttw::file::getFileTypeString(t->getPath());
    // char* test = new char[t->getFileSize()];
    // memcpy(test, t->getFileContext(), t->getFileSize());
    // auto t1 = new ttw::file::File(test,"test.json", t->getFileSize(), t->getFileMode());
    // LOG_DEBUG(logger_) << t1->getFileSize();
    // LOG_DEBUG(logger_) << t1->isValid();
    // LOG_DEBUG(logger_) << t1->getPath();
    // LOG_DEBUG(logger_) << ttw::file::getFileTypeString(t->getPath());
    // t1->save();
//home/ttw/code/ttw/tests/testjson.json
    // delete t;
    // delete t1;


    std::vector<std::string> v;
    //ttw::file::getFileAllList("/home/ttw/code/ttw/ttw", v, "ttw/");
    for(auto it = v.begin(); it != v.end(); ++it){
        LOG_DEBUG(logger_) << *it;
    }

    return 0;
}