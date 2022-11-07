cmake_minimum_required(VERSION 3.13.4)
project(tinylang)

set(CMAKE_CXX_STANDARD 17)

set(LLVM_DIR "/usr/local/aideqc/llvm/lib/cmake/llvm")

find_package(LLVM REQUIRED HINTS "${LLVM_INCLUDE_DIR}")
if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
    include(ChooseMSVCCRT)

    include(AddLLVM)
    include(HandleLLVMOptions)

    include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
    link_directories(${LLVM_LIBRARY_DIR})
endif ()

set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD}
        AggressiveInstCombine Analysis AsmParser
        BitWriter CodeGen Core Coroutines IPO IRReader
        InstCombine Instrumentation MC ObjCARCOpts Remarks
        ScalarOpts Support Target TransformUtils Vectorize
        Passes)

set(TINYLANG_BUILT_STANDALONE 1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
    message(FATAL_ERROR "In-source builds are not allowed. ")
endif()

set(TINYLANG_VERSION_STRING "0.1")
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tinylang/Basic/Version.inc.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/tinylang/Basic/Version.inc)

include(AddTinylang)

include_directories(BEFORE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)