cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(ping-cli VERSION 0.1.1 LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(src)

add_test(
  NAME
    ping-cli-valgrind-check
  COMMAND
    sh -c "sudo timeout --preserve-status -s 2 5 valgrind --error-exitcode=1 --leak-check=full --trace-children=yes ${CMAKE_SOURCE_DIR}/bin/ping-cli google.com 2>&1 | grep \"no leaks are possible\""
  )

add_test(
  NAME
    ping-cli-run
  COMMAND
    sudo timeout --preserve-status -s 2 5 ${CMAKE_SOURCE_DIR}/bin/ping-cli google.com
  )
