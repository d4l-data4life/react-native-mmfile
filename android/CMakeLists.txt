project(NitroMmfile)
cmake_minimum_required(VERSION 3.9.0)

set (PACKAGE_NAME NitroMmfile)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define C++ library and add all sources
add_library(${PACKAGE_NAME} SHARED
        src/main/cpp/cpp-adapter.cpp
        ../cpp/HybridMmfile.cpp
        ../cpp/HybridMmfileFactory.cpp
)

if (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        target_compile_options(${PACKAGE_NAME} PRIVATE -march=armv8-a+crypto -O2)
endif()

if (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        target_compile_options(${PACKAGE_NAME} PRIVATE -maes -msse4.2 -O2)
endif()

# Add Nitrogen specs :)
include(${CMAKE_SOURCE_DIR}/../nitrogen/generated/android/NitroMmfile+autolinking.cmake)

# Set up local includes
include_directories(
        "src/main/cpp"
        "../cpp"
)

find_library(LOG_LIB log)

# Link all libraries together
target_link_libraries(
        ${PACKAGE_NAME}
        ${LOG_LIB}
        android                                   # <-- Android core
)
