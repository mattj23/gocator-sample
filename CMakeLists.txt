cmake_minimum_required(VERSION 3.16)
project(gocator_record)

set(CMAKE_CXX_STANDARD 20)

find_library(GOCATOR NAMES GoSdk REQUIRED)
find_library(KAPI NAMES kApi REQUIRED)
include_directories(include)

set(SOURCE_FILES
        src/raw_message.cpp
        src/gocator.cpp)

# The main binary performs recording
add_executable(gocator_record src/main.cpp ${SOURCE_FILES})
target_link_libraries(gocator_record PUBLIC ${GOCATOR} ${KAPI})

# The convert binary prints out x, y, z, color points as an example of
# deserializing the files created by the recording binary
add_executable(gocator_convert src/gocator_convert.cpp ${SOURCE_FILES})
target_link_libraries(gocator_convert PUBLIC ${GOCATOR} ${KAPI})

