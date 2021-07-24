cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)
project(p2p)

find_package(CURL)
message("Found CURL_LIBRARIES: ${CURL_LIBRARIES}")

# Find all executables
# file(GLOB project_SRCS src/socket/*.h src/*.cpp src/*.h, ) #src/*.h
# message("ALL EXECUTABLES: ${project_SRCS}")

add_executable(p2p
    include/socket_creator.h
    src/socket_creator.cpp
    src/socket/socket_utils.h
    src/socket/socket_utils.h
    src/socket/ip_call.h
    src/socket/port.h

    src/main.cpp
)

target_include_directories(p2p
        PUBLIC
        include
        PRIVATE
        src)


# Add project executable
# add_executable(p2p ${project_SRCS})
target_link_libraries(p2p ${CURL_LIBRARIES})