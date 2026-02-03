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
# Note: This requires a specific Clang fork (e.g., bloomberg/clang-p2996)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fexperimental-library)
    message(STATUS "Clang detected. Adding -fexperimental-library for P2996 support.")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # GCC 16+ trunk support (if available)
    message(WARNING "Experimental Reflection on GCC is bleeding edge. Ensure you are using trunk.")
endif()

# ==============================================================================
# Dependencies (FetchContent)
# ==============================================================================
include(FetchContent)

# 1. Fetch GoogleTest
message(STATUS "Fetching GoogleTest...")
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. Fetch cURL
# We disable unnecessary protocols to speed up the build time
message(STATUS "Fetching cURL...")
set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(CURL_ENABLE_SSL ON CACHE INTERNAL "") # HTTPS is needed for Wikidata
set(HTTP_ONLY ON CACHE INTERNAL "")       # Disable FTP, LDAP, etc.

FetchContent_Declare(
  curl
  URL https://github.com/curl/curl/releases/download/curl-8_5_0/curl-8.5.0.tar.gz
)
FetchContent_MakeAvailable(curl)

# ==============================================================================
# Main Application
# ==============================================================================
add_executable(SparqReflect src/main.cpp)

# Link dependencies
target_link_libraries(SparqReflect 
    PRIVATE 
    CURL::libcurl
)

# Include directories (optional, usually handled by target_link_libraries)
target_include_directories(SparqReflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ==============================================================================
# Unit Tests
# ==============================================================================
enable_testing()

add_executable(unit_tests tests/test_main.cpp)

target_link_libraries(unit_tests
    PRIVATE
    GTest::gtest_main
    CURL::libcurl
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(unit_tests)