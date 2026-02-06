cmake_minimum_required(VERSION 3.25)

# Project Definition
project(SparqReflect
    VERSION 0.1.0
    LANGUAGES CXX
)

# ==============================================================================
# 1. Standard & Compiler Configuration
# ==============================================================================
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configuration for clang-p2996 (Experimental Reflection)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Flags required for P2996 features
    set(P2996_FLAGS 
        -stdlib=libc++ 
        -fexperimental-library 
        -freflection 
        -fexpansion-statements
    )

    add_compile_options(${P2996_FLAGS})
    add_link_options(-stdlib=libc++)

    # --- Runtime Linker Fix (RPATH) ---
    # Automagically find the custom libc++.so from the toolchain to avoid conflicts
    # with the system's older libc++.
    get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    get_filename_component(COMPILER_ROOT "${COMPILER_BIN_DIR}" DIRECTORY)

    # Search for libc++.so (it might be in lib/ or lib/x86_64-unknown-linux-gnu/)
    find_library(LIBCPP_PATH 
        NAMES "c++" 
        HINTS "${COMPILER_ROOT}/lib" 
              "${COMPILER_ROOT}/lib64"
              "${COMPILER_ROOT}/lib/x86_64-unknown-linux-gnu"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )

    if(LIBCPP_PATH)
        get_filename_component(TOOLCHAIN_LIB_DIR "${LIBCPP_PATH}" DIRECTORY)
        message(STATUS "Toolchain Libs Found: ${TOOLCHAIN_LIB_DIR}")
        
        # Bake library path into executables
        list(APPEND CMAKE_BUILD_RPATH "${TOOLCHAIN_LIB_DIR}")
        list(APPEND CMAKE_INSTALL_RPATH "${TOOLCHAIN_LIB_DIR}")
        link_directories(${TOOLCHAIN_LIB_DIR})
    else()
        message(WARNING "Could not find toolchain libc++. Manual LD_LIBRARY_PATH might be needed.")
    endif()
endif()

# ==============================================================================
# 2. Dependencies
# ==============================================================================
include(FetchContent)

# cURL (HTTP Client)
FetchContent_Declare(curl
    URL https://github.com/curl/curl/releases/download/curl-8_5_0/curl-8.5.0.tar.gz
)
set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
# Only build static libs for cURL to simplify linkage
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "") 
set(CURL_ENABLE_SSL ON CACHE INTERNAL "") 
FetchContent_MakeAvailable(curl)

# GoogleTest (Unit Testing)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Force STATIC build for GTest to avoid RPATH issues with libgtest.so
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ==============================================================================
# 3. Targets
# ==============================================================================

# --- Main Application ---
add_executable(SparqReflect src/main.cpp)

target_include_directories(SparqReflect PRIVATE src)
target_link_libraries(SparqReflect PRIVATE CURL::libcurl)

# --- Unit Tests ---
enable_testing()

add_executable(unit_tests tests/test_main.cpp)

# Allow tests to include headers from 'src'
target_include_directories(unit_tests PRIVATE src)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main CURL::libcurl)

# Setup Test Discovery
include(GoogleTest)

# Define environment for tests (fallback if RPATH fails)
if(TOOLCHAIN_LIB_DIR)
    set(TEST_ENV "LD_LIBRARY_PATH=${TOOLCHAIN_LIB_DIR}:$ENV{LD_LIBRARY_PATH}")
else()
    set(TEST_ENV "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}")
endif()

# DISCOVERY_MODE PRE_TEST is crucial for experimental toolchains to avoid build-time crashes
gtest_discover_tests(unit_tests 
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ENVIRONMENT "${TEST_ENV}"
)