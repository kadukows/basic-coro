cmake_minimum_required(VERSION 3.8)
project(basic-coro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -O3 -Wall")

set(BASICCORO_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BASICCORO_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(BASICCORO_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(BASICCORO_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/)


add_subdirectory(src)
add_subdirectory(test)
