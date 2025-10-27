cmake_minimum_required(VERSION 3.16)

project(ArenaX VERSION 0.1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

### EXTERNAL LIBRARIES ###
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

# Fetch Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.4
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Set warning flags based on compiler (after external libraries)
if(MSVC)
    set(WARNING_FLAGS /W4 /WX)
else()
    set(WARNING_FLAGS -Wall -Wextra -pedantic -Werror)
endif()

# ArenaX library (header-only)
add_library(arenax INTERFACE)
target_include_directories(arenax INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)



### TESTS ###
enable_testing()

# Arena tests
add_executable(test_arena tests/test_arena.cpp)
target_link_libraries(test_arena PRIVATE arenax GTest::gtest_main)
target_compile_options(test_arena PRIVATE ${WARNING_FLAGS})


# All tests combined
add_executable(test_all
    tests/test_arena.cpp
)
target_link_libraries(test_all PRIVATE arenax GTest::gtest_main)
target_compile_options(test_all PRIVATE ${WARNING_FLAGS})

# Add tests to CTest
add_test(NAME ArenaTests COMMAND test_arena)
add_test(NAME AllTests COMMAND test_all)



# Print build information
message(STATUS "")
message(STATUS "ArenaX Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
