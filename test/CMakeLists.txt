cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
project(tests)

find_package(GTest REQUIRED)
find_package(CURL)
find_package(Protobuf REQUIRED)
# set(GMOCK_LIBRARIES /usr/lib/libgmock.a /usr/lib/libgmock_main.a)

add_executable(tests 
    ../include/messages.h
    ../src/messages.cpp
    include/p2p_tests.h
    include/p2p_message_tests.h
    include/server_tests.h
    include/test_crypto_messages.h
    include/tests.h
    
    p2p_tests.cpp
    p2p_message_tests.cpp
    test_crypto_messages.cpp
    server_tests.cpp

    main.cpp
)

target_include_directories(tests
    PUBLIC
    include
    ../server
    ../include
)

target_link_libraries(tests
    cryptoRSA
    payload
    client
    server_lib
    ${GTEST_LIBRARIES} 
    ${GMOCK_LIBRARIES}
)

add_test(NAME tests_server COMMAND tests)