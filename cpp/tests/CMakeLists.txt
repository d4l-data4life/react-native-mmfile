# CMakeLists.txt for the tests directory
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(TurboModuleTests)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)

# Enable testing
enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Add gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Collect all test source files
set(TEST_SOURCES
    AESTest.cpp
    MMapFileTest.cpp
    MMapFileBench.cpp
    MMapEncryptedFileTest.cpp
)

# Create test executables dynamically
foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE GTest::GTest GTest::Main pthread)
    target_include_directories(${TEST_NAME} PRIVATE ..)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
