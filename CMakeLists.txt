cmake_minimum_required(VERSION 3.25)

# Project Definition
project(SparqReflect
    VERSION 0.1.0
    LANGUAGES CXX
)

# ==============================================================================
# C++ Standard Configuration (C++26 / P2996)
# ==============================================================================
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags for Experimental Reflection (P2996)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # IMPORTANT FIXES:
    # 1. -stdlib=libc++: Required for the experimental standard library.
    # 2. -fexperimental-library: Unlocks experimental headers.
    # 3. -freflection: Enables the reflection operator (^T).
    # 4. -fexpansion-statements: Enables 'template for' loops.
    add_compile_options(
        -stdlib=libc++ 
        -fexperimental-library 
        -freflection 
        -fexpansion-statements
    )
    
    # Linker must also use libc++
    add_link_options(-stdlib=libc++)

    # [FIX] Runtime Linker Issue (Symbol Lookup Error):
    # The app fails because it loads the SYSTEM libc++.so instead of the TOOLCHAIN one.
    # We must find the EXACT directory containing the toolchain's libc++.so
    
    # 1. Detect toolchain root
    get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(COMPILER_ROOT_DIR ${COMPILER_BIN_DIR} DIRECTORY)
    
    # 2. Search for libc++.so (it might be in lib/ or lib/x86_64-unknown-linux-gnu/)
    find_library(LIBCPP_PATH 
        NAMES "c++" 
        HINTS "${COMPILER_ROOT_DIR}/lib" 
              "${COMPILER_ROOT_DIR}/lib64"
              "${COMPILER_ROOT_DIR}/lib/x86_64-unknown-linux-gnu"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )

    if(LIBCPP_PATH)
        get_filename_component(TOOLCHAIN_LIB_DIR ${LIBCPP_PATH} DIRECTORY)
        message(STATUS " [FIX] Found Toolchain Lib Dir: ${TOOLCHAIN_LIB_DIR}")
        
        # 3. Force CMake to add this directory to RPATH during build & install
        list(APPEND CMAKE_BUILD_RPATH "${TOOLCHAIN_LIB_DIR}")
        list(APPEND CMAKE_INSTALL_RPATH "${TOOLCHAIN_LIB_DIR}")
        
        # 4. Help the linker find libraries at link time
        link_directories(${TOOLCHAIN_LIB_DIR})
        
        message(STATUS " [FIX] RPATH configured. Executables will search ${TOOLCHAIN_LIB_DIR} for shared libraries.")
    else()
        message(WARNING "Could not find libc++.so relative to ${COMPILER_ROOT_DIR}. Manual LD_LIBRARY_PATH may be required.")
    endif()
    
    message(STATUS "Clang detected. Enabled P2996 reflection flags.")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(WARNING "Experimental Reflection on GCC is bleeding edge. Ensure you are using trunk.")
endif()

# ==============================================================================
# Dependencies (FetchContent)
# ==============================================================================
include(FetchContent)

# 1. Fetch GoogleTest (RESTORED)
message(STATUS "Fetching GoogleTest...")
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Force GTest to use shared CRT to match standard usage
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. Fetch cURL
message(STATUS "Fetching cURL...")
set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(CURL_ENABLE_SSL ON CACHE INTERNAL "") 
set(HTTP_ONLY ON CACHE INTERNAL "")       

FetchContent_Declare(
  curl
  URL https://github.com/curl/curl/releases/download/curl-8_5_0/curl-8.5.0.tar.gz
)
FetchContent_MakeAvailable(curl)

# ==============================================================================
# Main Application
# ==============================================================================
add_executable(SparqReflect src/main.cpp)

target_link_libraries(SparqReflect 
    PRIVATE 
    CURL::libcurl
)

target_include_directories(SparqReflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ==============================================================================
# Unit Tests (RESTORED)
# ==============================================================================
enable_testing()

add_executable(unit_tests tests/test_main.cpp)

# Include main source directory to test headers
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(unit_tests
    PRIVATE
    GTest::gtest_main
    CURL::libcurl
)

include(GoogleTest)

# [FIX] DISCOVERY_MODE PRE_TEST + ENVIRONMENT
# 1. Delays discovery to execution time so build doesn't fail.
# 2. Sets LD_LIBRARY_PATH explicitly for the test runner environment.
if(TOOLCHAIN_LIB_DIR)
    set(TEST_ENV "LD_LIBRARY_PATH=${TOOLCHAIN_LIB_DIR}:$ENV{LD_LIBRARY_PATH}")
else()
    set(TEST_ENV "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}")
endif()

gtest_discover_tests(unit_tests 
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ENVIRONMENT "${TEST_ENV}"
)