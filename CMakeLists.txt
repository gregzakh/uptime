cmake_minimum_required(VERSION 3.20)

if (APPLE)
   execute_process(
      COMMAND xcrun --show-sdk-path
      OUTPUT_VARIABLE SDK_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
   )

   if (SDK_PATH)
      set(CMAKE_CXX_FLAGS_INIT "-isysroot ${SDK_PATH}")
      set(CMAKE_EXE_LINKER_FLAGS_INIT "-isysroot ${SDK_PATH}")
   else()
      message(FATAL_ERROR "Failed to detect SDKROOT via xcrun.")
   endif()

   execute_process(
      COMMAND which g++-15
      OUTPUT_VARIABLE GPP_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
   )

   if (GPP_PATH)
      set(CMAKE_CXX_COMPILER "${GPP_PATH}" CACHE STRING "C++ compiler" FORCE)
   else()
      message(FATAL_ERROR "g++-15 not found, use 'brew' to install it.")
   endif()
endif()

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
