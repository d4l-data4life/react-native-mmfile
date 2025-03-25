# CMakeLists.txt for the tests directory
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(TurboModuleTests)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)        # Use C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)

# Enable testing
enable_testing()

# Add gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executables
add_executable(AESTest AESTest.cpp ../AES.h)
add_executable(MMapFileTest MMapFileTest.cpp)
add_executable(MMapEncryptedFileTest MMapEncryptedFileTest.cpp)

# Link the test executables with the necessary libraries
target_link_libraries(AESTest PRIVATE ${GTEST_LIBRARY_DEBUG} ${GTEST_MAIN_LIBRARY_DEBUG} pthread)
target_link_libraries(MMapFileTest PRIVATE ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
target_link_libraries(MMapEncryptedFileTest PRIVATE ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Include the source files for testing
target_include_directories(AESTest PRIVATE ..)
target_include_directories(MMapFileTest PRIVATE ..)
target_include_directories(MMapEncryptedFileTest PRIVATE ..)

# Add tests
add_test(NAME AESTest COMMAND AESTest)
add_test(NAME MMapFileTest COMMAND MMapFileTest)
add_test(NAME MMapEncryptedFileTest COMMAND MMapEncryptedFileTest)
