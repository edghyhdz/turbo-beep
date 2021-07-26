cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(p2p)
enable_testing()

find_package(CURL)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

add_executable(p2p
    include/socket_creator.h
    protos/payload.pb.h
    protos/payload.pb.cc
    protos/payload.proto
    src/socket_creator.cpp
    src/socket/socket_utils.h
    src/socket/socket_utils.h
    src/socket/ip_call.h
    src/socket/port.h
    src/main.cpp
)

include_directories(protos)

target_include_directories(p2p
        PUBLIC
        include
        PRIVATE
        protos
        src)

add_subdirectory(server)
add_subdirectory(test)

target_link_libraries(p2p 
    ${CURL_LIBRARIES} 
    ${PROTOBUF_LIBRARY}
    Threads::Threads
)