cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(p2p)
enable_testing()

find_package(CURL)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(p2p
    include/messages.h
    src/messages.cpp
    src/main.cpp
)

# Client library
add_library(client
    include/socket_creator.h
    src/socket_creator.cpp
    src/socket/socket_utils.h
    src/socket/socket_utils.h
    src/socket/ip_call.h
    src/socket/port.h
)

# Crypto
add_library(cryptoRSA
    include/crypto.h
    src/crypto.cpp
)

# Protobuf payload library
add_library(payload
    protos/payload.pb.h
    protos/payload.pb.cc
    protos/payload.proto
)

include_directories(protos)

target_include_directories(p2p
        PUBLIC
        include
        PRIVATE
        protos
        src
)

target_include_directories(client
    PUBLIC
    include
    PRIVATE
    src
)

target_include_directories(cryptoRSA
        PUBLIC
        include
        PRIVATE
        protos
        src
)

add_subdirectory(server)
add_subdirectory(test)

target_link_libraries(cryptoRSA
    OpenSSL::SSL     
)

target_link_libraries(payload
    ${PROTOBUF_LIBRARY}
)

target_link_libraries(client
    ${CURL_LIBRARIES} 
    Threads::Threads
)

target_link_libraries(p2p 
    payload
    client
)