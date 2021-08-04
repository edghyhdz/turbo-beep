cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(server)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

add_executable(server
    ../include/messages.h
    ../src/messages.cpp
    main.cpp
)

add_library(server_lib
    server.h
    server.cpp
)

target_include_directories(server_lib
        PUBLIC
        ../protos/
        ../include/
        ../src/
)

target_link_libraries(server 
    server_lib
    cryptoRSA
    payload
    Threads::Threads
)