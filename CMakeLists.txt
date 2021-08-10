cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(p2p)
enable_testing()

find_package(CURL)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(p2p
    ${PROJECT_SOURCE_DIR}/include/vendor/base64.h
    ${PROJECT_SOURCE_DIR}/include/vendor/base64.cpp
    ${PROJECT_SOURCE_DIR}/include/messages.h
    ${PROJECT_SOURCE_DIR}/src/messages.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)

# Pass CERTIFICATES_PATH variable to SetPath.cmake
set(CERTIFICATES_PATH ${CMAKE_CURRENT_BINARY_DIR}/certs/)

add_custom_command(TARGET p2p PRE_BUILD 
        COMMAND ${CMAKE_COMMAND} 
        -DCERTIFICATES_PATH=${CERTIFICATES_PATH} 
        -P ${CMAKE_CURRENT_SOURCE_DIR}/SetPath.cmake)

file(COPY ${CMAKE_SOURCE_DIR}/certs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Client library
add_library(client
    ${PROJECT_SOURCE_DIR}/include/peer.h
    ${PROJECT_SOURCE_DIR}/src/peer.cpp
    ${PROJECT_SOURCE_DIR}/src/socket/socket_utils.h
    ${PROJECT_SOURCE_DIR}/src/socket/socket_utils.h
    ${PROJECT_SOURCE_DIR}/src/socket/ip_call.h
    ${PROJECT_SOURCE_DIR}/src/socket/port.h
)

# Crypto
add_library(cryptoRSA
    ${PROJECT_SOURCE_DIR}/include/crypto.h
    ${PROJECT_SOURCE_DIR}/src/crypto.cpp
)

# Protobuf payload library
add_library(payload
    ${PROJECT_SOURCE_DIR}/protos/payload.pb.h
    ${PROJECT_SOURCE_DIR}/protos/payload.pb.cc
    ${PROJECT_SOURCE_DIR}/protos/payload.proto
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
    cryptoRSA
    ${CURL_LIBRARIES} 
    Threads::Threads
)

target_link_libraries(p2p 
    payload
    client
)