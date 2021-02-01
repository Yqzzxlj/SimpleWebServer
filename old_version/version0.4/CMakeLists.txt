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
        util.cc
        Buffer.cc
        Epoll.cc
        Socket.cc
        HttpRequest.cc
        HttpResponse.cc
        HttpRequestParser.cc
        Server.cc
        main.cc
        )

add_executable(webserver ${SOURCE_FILES})
