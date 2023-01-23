cmake_minimum_required(VERSION 3.16s)
project(TP1)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -march=native -pthread")

set(PLATFORM_SPECIFIC_LIBS "-lpthread")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(inc)

set(SRCS
    main.cpp
)


find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(tp1 ${SRCS})



target_link_libraries(tp1 ${OpenCV_LIBS} )