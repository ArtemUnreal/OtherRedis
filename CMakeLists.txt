cmake_minimum_required(VERSION 3.10)
project(redis_killer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON) # CMAKE_VERBOSE_MAKEFILE — это переменная в CMake, 
# которая позволяет включить подробный вывод при сборке проекта с помощью Makefile

include(FetchContent)  # позволяет скачать библиотеки, если нет таковых

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.9.2
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)


set(SERVER_SOURCES src/server.cpp src/main.cpp)
set(CLIENT_SOURCES src/client.cpp)
set(TEST_SOURCES tests/test.cpp)

set(HEADERS src/common.h src/server.h)

add_executable(server ${SERVER_SOURCES} ${HEADERS})
add_executable(client ${CLIENT_SOURCES} ${HEADERS})

target_link_libraries(server pthread spdlog::spdlog)
target_link_libraries(client pthread spdlog::spdlog)

enable_testing()

add_executable(runTests ${TEST_SOURCES})
target_include_directories(runTests PRIVATE src)
target_compile_definitions(runTests PRIVATE TEST_BUILD)
target_link_libraries(runTests gtest gtest_main pthread spdlog::spdlog)

include_directories(src)