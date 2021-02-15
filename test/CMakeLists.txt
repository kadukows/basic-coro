cmake_minimum_required(VERSION 3.8)
project(basic-coro-tests)

add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)

set(TESTS_SOURCE_FILES
    main.cpp
    tests/AwaitableTaskTests.cpp
)

add_executable(basiccoro-tests ${TESTS_SOURCE_FILES})
target_include_directories(basiccoro-tests PUBLIC ${BASICCORO_HEADERS_DIR})
target_link_libraries(basiccoro-tests basiccoro gtest gmock)
add_dependencies(basiccoro-tests basiccoro)

add_custom_target(run-basiccoro-tests basiccoro-tests)
