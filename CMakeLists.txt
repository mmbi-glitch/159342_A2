cmake_minimum_required(VERSION 3.24)
project(159342_A2)

set(CMAKE_CXX_STANDARD 14)

set(BOOST_ROOT "C:/boost_1_82_0")

find_package(Boost 1.82.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(159342_A2_client secure_client/secure_client.cpp RsaSystem.cpp RsaSystem.h)
add_executable(159342_A2_server secure_server/secure_server.cpp RsaSystem.cpp RsaSystem.h)

target_link_libraries(159342_A2_client ws2_32)
target_link_libraries(159342_A2_server ws2_32)