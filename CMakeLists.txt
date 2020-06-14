cmake_minimum_required(VERSION 3.5)
project(Judger)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/include)
include_directories(utils)
include_directories(http)
include_directories(json)

add_executable(
        Judger
        judge_server.cpp
        utils/PropertiesReader.cpp
        utils/RedisConnector.cpp
        utils/compare.cpp
        judge_option.cpp
        judge_runner.cpp)

target_link_libraries(Judger /usr/local/lib/libhiredis.a)