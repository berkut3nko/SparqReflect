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
    # The error "_ZNSt3__113__hash_memoryEPKvm" means the app is finding an old system libc++.so
    # instead of the new one from the toolchain.
    # We must enforce RPATH in the BUILD tree so the binary works.
    
    # 1. Detect toolchain root
    get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(COMPILER_ROOT_DIR ${COMPILER_BIN_DIR} DIRECTORY)
    set(TOOLCHAIN_LIB_DIR "${COMPILER_ROOT_DIR}/lib")
    
    if(EXISTS "${TOOLCHAIN_LIB_DIR}")
        message(STATUS " [FIX] Detected Toolchain Lib Dir: ${TOOLCHAIN_LIB_DIR}")
        
        # 2. Force CMake to add this directory to RPATH during build & install
        # This ensures ./SparqReflect knows where to find the custom libc++.so
        list(APPEND CMAKE_BUILD_RPATH "${TOOLCHAIN_LIB_DIR}")
        list(APPEND CMAKE_INSTALL_RPATH "${TOOLCHAIN_LIB_DIR}")
        
        # 3. Help the linker find libraries at link time
        link_directories(${TOOLCHAIN_LIB_DIR})
        
        message(STATUS " [FIX] RPATH configured. The executable will search ${TOOLCHAIN_LIB_DIR} for shared libraries.")
    else()
        message(WARNING "Could not auto-detect toolchain lib dir at ${TOOLCHAIN_LIB_DIR}. \
        Please export LD_LIBRARY_PATH=/path/to/clang-p2996/build/lib")
    endif()
    
    message(STATUS "Clang detected. Enabled P2996 reflection flags.")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(WARNING "Experimental Reflection on GCC is bleeding edge. Ensure you are using trunk.")
endif()

# ==============================================================================
# Dependencies (FetchContent)
# ==============================================================================
include(FetchContent)

# 1. Fetch cURL
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