cmake_minimum_required(VERSION 3.16)
project(tinywebserver)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS
        -Wall
        -std=c++11
        -Wno-unused-parameter
        -O3
        )

link_libraries(pthread)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SOURCE_FILES
        Timer.cc
        Util.cc
        Buffer.cc
        Epoll.cc
        Socket.cc
        http/HttpRequest.cc
        http/HttpResponse.cc
        http/HttpRequestParser.cc
        http/HttpData.cc
        http/HttpServer.cc
        log/AsyncLogging.cc
        log/FileUtil.cc
        log/LogFile.cc
        log/Logging.cc
        log/LogStream.cc
        main.cc
        )

add_executable(webserver ${SOURCE_FILES})
