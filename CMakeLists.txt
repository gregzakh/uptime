cmake_minimum_required(VERSION 3.20)

project(uptime
   VERSION 1.0
   DESCRIPTION "Simple uptime utility"
   LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES uptime.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
