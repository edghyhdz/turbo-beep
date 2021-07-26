cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(tests)

find_package(GTest REQUIRED)
find_package(CURL)
set(GMOCK_LIBRARIES /usr/lib/libgmock.a /usr/lib/libgmock_main.a)


add_executable(tests 
    ../server/server.h 
    ../server/server.cpp 
    ../src/socket/ip_call.h
    ../src/socket/port.h
    ../src/socket/socket_utils.h
    ../include/socket_creator.h
    ../src/socket_creator.cpp
    include/p2p_tests.h
    include/server_tests.h
    include/tests.h
    p2p_tests.cpp
    server_tests.cpp

    main.cpp
)

target_include_directories(tests
        PUBLIC
        include
        ../server
        ../include
        ../src
        ../src/socket
)

target_link_libraries(tests
    ${GTEST_LIBRARIES} 
    ${GMOCK_LIBRARIES}
    ${CURL_LIBRARIES} 
    pthread
)

add_test(NAME tests_server COMMAND tests)