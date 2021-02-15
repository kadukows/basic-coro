cmake_minimum_required(VERSION 3.8)
project(basic-coro)

set(BASICCORO_SOURCE_FILES
    basiccoro/SingleEvent.cpp)

set(BASICCORO_HEADER_FILES
    basiccoro/SingleEvent.hpp
    basiccoro/AwaitableTask.hpp)

add_library(basiccoro STATIC ${BASICCORO_SOURCE_FILES})
install(TARGETS basiccoro DESTINATION ${BASICCORO_INSTALL_LIB_DIR})
install(FILES ${BASICCORO_HEADER_FILES} DESTINATION ${BASICCORO_INSTALL_INCLUDE_DIR})

add_executable(basiccoro-main main.cpp)
target_link_libraries(basiccoro-main basiccoro)
target_include_directories(basiccoro-main PUBLIC ${PROJECT_SOURCE_DIR})
install(TARGETS basiccoro-main DESTINATION ${BASICCORO_INSTALL_BIN_DIR})
