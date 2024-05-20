cmake_minimum_required(VERSION 3.10)
project(redis_killer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON) # CMAKE_VERBOSE_MAKEFILE — это переменная в CMake, 
# которая позволяет включить подробный вывод при сборке проекта с помощью Makefile

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

set(SERVER_SOURCES src/server.cpp)
set(CLIENT_SOURCES src/client.cpp)
set(TEST_SOURCES tests/test.cpp)

set(HEADERS src/common.h)

add_executable(server ${SERVER_SOURCES} ${HEADERS})
add_executable(client ${CLIENT_SOURCES} ${HEADERS})

target_link_libraries(server pthread)
target_link_libraries(client pthread)

enable_testing()
add_executable(runTests ${TEST_SOURCES})
target_link_libraries(runTests gtest gtest_main pthread)

include_directories(src)