cmake_minimum_required(VERSION 3.5)
project(tpchjoinoptimized C CXX)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)

# Compiler flags for the different targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fPIC -Wall -Wextra -fno-omit-frame-pointer -march=native -Wno-unknown-pragmas")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fPIC -Wall -Wextra -fno-omit-frame-pointer -march=native")

find_package(Threads)

include(lib/gtest.cmake)

### Library for join query
add_library(sum src/sum.cpp)
target_include_directories(sum PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

### Tests
add_executable(test_all test/test_sum.cpp)
target_link_libraries(test_all sum gtest gtest_main pthread)

### main executable
add_executable(main src/main.cpp)
target_link_libraries(main sum)
