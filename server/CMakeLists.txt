cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(server)

add_executable(server
    server.h
    server.cpp
    main.cpp
)
