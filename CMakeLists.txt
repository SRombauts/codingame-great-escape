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
    set(CPPLINT_ARG_OUTPUT      "--output=vs7")
    set(CPPCHECK_ARG_TEMPLATE   "--template=vs")
    set(DEV_NULL                "NUL")
    set(SYSTEM_LIBRARIES        "")
    # Set warning level to maximum (instead of default /W3)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else (MSVC)
    set(CPPLINT_ARG_OUTPUT      "--output=eclipse")
    set(CPPCHECK_ARG_TEMPLATE   "--template=gcc")
    set(DEV_NULL                "/dev/null")
    set(SYSTEM_LIBRARIES        "rt")

    # C++11 :
    add_definitions(-std=c++0x) # equivalent to "-std=c++11" but backward compatible for GCC 4.6 on Travic-CI
    # Maximum of Warnings :
    if (CMAKE_COMPILER_IS_GNUCXX)
        # GCC flags
        add_definitions(-rdynamic -fstack-protector-all -Wall -Wextra -Wswitch-default -Wswitch-enum -Winit-self -Wformat-security -Wfloat-equal -Wcast-qual -Wconversion -Wlogical-op -Winline)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9")
            add_definitions (-Wfloat-conversion)
            add_definitions (-Wshadow)
        endif ()
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # Clang flags
        add_definitions(-fstack-protector-all -Wall -Wextra -Wswitch-default -Wswitch-enum -Winit-self -Wformat-security -Wfloat-equal -Wshadow -Wcast-qual -Wconversion -Winline)
    endif (CMAKE_COMPILER_IS_GNUCXX)
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
