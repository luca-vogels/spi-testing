cmake_minimum_required(VERSION 3.9)

project(testing)

# ---------------------------------------------------------------------------------------------------------------------
# --- [ SETTINGS ] ----------------------------------------------------------------------------------------------------

# How this project should be build. Possible values are:
# - Release     Best performance without additional overhead
# - Debug       Adds debug infos and enables sanitizers
set(CMAKE_BUILD_TYPE "Release")


# Compiler Options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX -std=c++23 -pthread -Wall -Wextra -Wconversion -pedantic")


# Path where the CMake modules are located to find all required libraries 
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Path where the binaries should be compiled into
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# --- [ END SETTINGS ] ------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(STATUS "BUILD TYPE:        " ${CMAKE_BUILD_TYPE} "  (not suitable for release)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX -DDEBUG")

    # ThreadSanitizer
    set(CMAKE_C_FLAGS_TSAN "-fsanitize=thread -g -O1" CACHE STRING "Flags used by the C compiler during ThreadSanitizer builds." FORCE)
    set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -g -O1" CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds." FORCE)

    # AddressSanitize
    set(CMAKE_C_FLAGS_ASAN "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C compiler during AddressSanitizer builds." FORCE)
    set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds." FORCE)

    # LeakSanitizer
    set(CMAKE_C_FLAGS_LSAN "-fsanitize=leak -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C compiler during LeakSanitizer builds." FORCE)
    set(CMAKE_CXX_FLAGS_LSAN "-fsanitize=leak -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C++ compiler during LeakSanitizer builds." FORCE)

    # MemorySanitizer
    set(CMAKE_C_FLAGS_MSAN "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2" CACHE STRING "Flags used by the C compiler during MemorySanitizer builds." FORCE)
    set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2" CACHE STRING "Flags used by the C++ compiler during MemorySanitizer builds." FORCE)

    # UndefinedBehaviour
    set(CMAKE_C_FLAGS_UBSAN "-fsanitize=undefined" CACHE STRING "Flags used by the C compiler during UndefinedBehaviourSanitizer builds." FORCE)
    set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined" CACHE STRING "Flags used by the C++ compiler during UndefinedBehaviourSanitizer builds." FORCE)
else()
    MESSAGE(STATUS "BUILD TYPE:       " ${CMAKE_BUILD_TYPE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()



# the compiler flags for compiling C++ sources
MESSAGE( STATUS "CMAKE_CXX_FLAGS:  " ${CMAKE_CXX_FLAGS} )

add_subdirectory(src)
