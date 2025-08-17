cmake_minimum_required(VERSION 3.15)
project(kvstorage_ttl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# библиотека (заголовочная)
add_library(kvstorage INTERFACE)
target_include_directories(kvstorage INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# пример
add_executable(kvstorage_example src/main.cpp)
target_link_libraries(kvstorage_example kvstorage)

# ---- GoogleTest ----
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(kvstorage_tests
    tests/tests.cpp
)
target_link_libraries(kvstorage_tests
    kvstorage
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(kvstorage_tests)
