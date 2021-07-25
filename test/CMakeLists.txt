cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
project(tests)

find_package(GTest REQUIRED)
set(GMOCK_LIBRARIES /usr/lib/libgmock.a /usr/lib/libgmock_main.a)

add_executable(tests tests.cpp)

target_link_libraries(tests
    ${GTEST_LIBRARIES} 
    ${GMOCK_LIBRARIES} 
    pthread
)

add_test(NAME tests_p2p COMMAND tests)