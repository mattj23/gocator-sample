cmake_minimum_required(VERSION 3.24)
project(conversion)

set(CMAKE_CXX_STANDARD 20)

find_package(Open3D REQUIRED)
include_directories(include)

set(SOURCE_INCLUDES
        src/main.cpp
        src/raw_message.cpp)

add_executable(conversion ${SOURCE_INCLUDES})
target_include_directories(conversion PUBLIC ${Open3D_INCLUDE_DIRS} )
target_link_libraries(conversion  ${Open3D_LIBRARIES})
