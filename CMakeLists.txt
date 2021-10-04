cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(TASTE-Linux-Drivers VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build/lib)
set(TESTS_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build/tests)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(MessageColors)
include(LoggingHelper)

add_library(common_build_options INTERFACE)
include(CommonBuildOptions)
include(Sanitizers)

option(TASTE_RUNTIME_COMMON_WARNINGS_AS_ERRORS
       "Treat compiler wargnings as errors"
       TRUE)

if(OPTIONS_WARNINGS_AS_ERRORS)
    log_option_enabled("warnings as errors")
    set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
endif()

set(CLANG_WARNINGS -Wall
                   -Wextra
                   -Wpedantic
                   -Wshadow
                   -Wcast-align
                   -Wunused
                   -Wsign-conversion
                   -Wnull-dereference
                   -Wdouble-promotion
                   -Wformat=2)

set(GCC_WARNINGS ${CLANG_WARNINGS}
                 -Wmisleading-indentation
                 -Wduplicated-cond
                 -Wduplicated-branches
                 -Wlogical-op)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  endif()

  target_compile_options(common_build_options INTERFACE ${PROJECT_WARNINGS})

include(CppCheck)
include(ClangTidy)
include(ClangFormat)
include(Doxygen)
  
add_subdirectory(src)
