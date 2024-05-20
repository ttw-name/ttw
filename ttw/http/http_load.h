

#ifndef __TTW__HTTP_LOAD
#define __TTW__HTTP_LOAD

#include "http_file.h"
#include "servlet.h"
#include "file/file.h"
#include "file/filecache.h"

namespace ttw{
namespace http{
    void LoadFile(ServletDispatch::ptr dispatch, file::FileCacheMan::ptr cache, const std::string& path, const std::vector<std::string>& losedirs);

}
}


#endif