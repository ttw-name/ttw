cmake_minimum_required(VERSION 3.4.1)
project(ttw)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -std=c++17 -g  -Wall -Werror -Wno-pessimizing-move -fpermissive -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -std=c17 -g -Wall -Werror -fpermissive -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


include_directories(.)
include_directories(ttw)
include_directories(ttw/db)
include_directories(ttw/json)
include_directories(ttw/http)
include_directories(ttw/file)

set(LIB_SRC
    ttw/ttw.cc
    ttw/address.cc
    ttw/config.cc
    ttw/coroutine.cc
    ttw/corscheduler.cc
    ttw/endian.cc
    ttw/ioscheduler.cc
    ttw/log.cc
    ttw/socket.cc
    ttw/tcpserver.cc
    ttw/timer.cc
    ttw/ttw.cc
    ttw/util.cc
    ttw/daemon.cc
#    ttw/db/mysqlconn.cc
    ttw/file/file.cc
    ttw/file/filecache.cc
    ttw/http/http_file.cc
    ttw/http/http_load.cc
    ttw/http/http_parser.cc
    ttw/http/httpclient_parser.cc
    ttw/http/http11_parser.cc
    ttw/http/http_session.cc
    ttw/http/http_sessiondata.cc
    ttw/http/http.cc
    ttw/http/httpserver.cc
    ttw/http/servlet.cc
    ttw/db/mysqlcppconn.cc
    ttw/json/jsonelement.cc
    ttw/json/reader.cc
)

set(LIB_O
    ttw
    #jsoncpp
    pthread
    mysqlcppconn
)

add_library(ttw SHARED ${LIB_SRC})
#force_redefine_file_macro_for_sources(ttw)

# add_executable(test_log tests/test_log.cc)
# add_dependencies(test_log ttw)
# target_link_libraries(test_log  ${LIB_O})

# add_executable(test_config tests/test_config.cc)
# add_dependencies(test_config ttw)
# target_link_libraries(test_config  ${LIB_O})

# add_executable(test_thread tests/test_thread.cc)
# add_dependencies(test_thread ttw)
# target_link_libraries(test_thread  ${LIB_O})

# add_executable(test_sch tests/test_sc_timer.cc)
# add_dependencies(test_sch ttw)
# target_link_libraries(test_sch  ${LIB_O})

# add_executable(test_cppconn tests/test_cppconn.cc)
# add_dependencies(test_cppconn ttw)
# target_link_libraries(test_cppconn  ${LIB_O})

# add_executable(test_config tests/test_config.cc)
# add_dependencies(test_config ttw01)
# target_link_libraries(test_config  ${LIB_O})

add_executable(test_web tests/web.cc)
add_dependencies(test_web ttw)
target_link_libraries(test_web  ${LIB_O})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
