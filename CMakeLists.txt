cmake_minimum_required(VERSION 3.10)
project(redis_killer)

set(CMAKE_CXX_STANDARD 17)

set(SERVER_SOURCES src/server.cpp)
set(CLIENT_SOURCES src/client.cpp)

set(HEADERS src/common.h)

add_executable(server ${SERVER_SOURCES} ${HEADERS})
add_executable(client ${CLIENT_SOURCES} ${HEADERS})

target_link_libraries(server pthread)
target_link_libraries(client pthread)