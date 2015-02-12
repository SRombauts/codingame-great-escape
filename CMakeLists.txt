# Copyright (c) 2015 Sébastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 2.6)
project(TheGreatEscape)

# Print some standard CMake variables
message(STATUS "CMAKE_SYSTEM_NAME/_VERSION      '${CMAKE_SYSTEM_NAME}' '${CMAKE_SYSTEM_VERSION}'")
message(STATUS "CMAKE_CXX_COMPILER_ID/_VERSION  '${CMAKE_CXX_COMPILER_ID}' '${CMAKE_CXX_COMPILER_VERSION}'")
if (NOT MSVC)
    message(STATUS "CMAKE_BUILD_TYPE                '${CMAKE_BUILD_TYPE}'")
endif (NOT MSVC)

# Define useful variables to handle OS/Compiler differences
if (MSVC)
    # Flags for linking with multithread static C and C++ runtime
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CPPLINT_ARG_OUTPUT      "--output=vs7")
    set(CPPCHECK_ARG_TEMPLATE   "--template=vs")
    set(DEV_NULL                "NUL")
    set(SYSTEM_LIBRARIES        "")
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
else (MSVC)
    message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

    set(CPPLINT_ARG_OUTPUT      "--output=eclipse")
    set(CPPCHECK_ARG_TEMPLATE   "--template=gcc")
    set(DEV_NULL                "/dev/null")
    set(SYSTEM_LIBRARIES        "rt")
    add_definitions(-std=c++0x)  # -std=c++11
endif (MSVC)
set(CPPLINT_ARG_LINELENGTH "--linelength=120")
set(CPPLINT_ARG_VERBOSE    "--verbose=2")

# List all sources/headers files
set(all_source_files
 ${CMAKE_SOURCE_DIR}/src/Main.cpp
)

# All includes are relative to the "src" directory
include_directories("${PROJECT_SOURCE_DIR}/src")

# add the application executable
add_executable(TheGreatEscape ${all_source_files})
target_link_libraries(TheGreatEscape ${SYSTEM_LIBRARIES})


# Optional additional targets:

option(THEGREATESCAPE_RUN_CPPLINT "Run cpplint.py tool for Google C++ StyleGuide." ON)
if (THEGREATESCAPE_RUN_CPPLINT)
    # add a cpplint target to the "all" target
    add_custom_target(cpplint
     ALL
     COMMAND python ${PROJECT_SOURCE_DIR}/cpplint/cpplint.py ${CPPLINT_ARG_OUTPUT} ${CPPLINT_ARG_LINELENGTH} ${CPPLINT_ARG_VERBOSE} ${all_source_files}
    )
else (THEGREATESCAPE_RUN_CPPLINT)
    message(STATUS "THEGREATESCAPE_RUN_CPPLINT OFF")
endif (THEGREATESCAPE_RUN_CPPLINT)

option(THEGREATESCAPE_RUN_CPPCHECK "Run cppcheck C++ static analysis tool." ON)
if (THEGREATESCAPE_RUN_CPPCHECK)
    # add a cppcheck target to the "all" target
    add_custom_target(cppcheck
     ALL
     COMMAND cppcheck -j 4 cppcheck --enable=style --quiet ${CPPCHECK_ARG_TEMPLATE} ${PROJECT_SOURCE_DIR}/src
    )
else (THEGREATESCAPE_RUN_CPPCHECK)
    message(STATUS "THEGREATESCAPE_RUN_CPPCHECK OFF")
endif (THEGREATESCAPE_RUN_CPPCHECK)

option(THEGREATESCAPE_RUN_DOXYGEN "Run Doxygen C++ documentation tool." ON)
if (THEGREATESCAPE_RUN_DOXYGEN)
    # add a Doxygen target to the "all" target
    if (NOT DEFINED ENV{TRAVIS})
        # if not runing on a Travis CI Virtual Machine
        add_custom_target(doxygen
         ALL
         COMMAND doxygen Doxyfile > ${DEV_NULL}
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    else()
        # but no Doxygen under Travis CI: too costly and no real benefit
        message("no Doxygen target when TRAVIS is defined")
    endif()
else (THEGREATESCAPE_RUN_DOXYGEN)
    message(STATUS "THEGREATESCAPE_RUN_DOXYGEN OFF")
endif (THEGREATESCAPE_RUN_DOXYGEN)
