cmake_minimum_required(VERSION 3.16)
project(gocator_record)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

find_library(GOCATOR NAMES GoSdk REQUIRED)
find_library(KAPI NAMES kApi REQUIRED)

set(SOURCE_FILES
        src/main.cpp
        src/raw_message.cpp
        src/gocator.cpp
        )

add_executable(gocator_record ${SOURCE_FILES})
target_link_libraries(gocator_record PUBLIC ${GOCATOR} ${KAPI})
