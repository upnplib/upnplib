# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-17

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(PTHREADS4W DESCRIPTION "POSIX Threads for Windows")

# This project is only installed with Microsoft Windows.
# It contains all settings to configure and build the POSIX Threads for Windows.
# It is independent from building other targets and can be run standalone
# (EXCLUDE_FROM_ALL). So you are able to provide the pthreads4w library before
# installing the main project without involving it. You should run its tests at
# least one time from the CMAKE_SOURCE_DIR:
# cmake -S pthreads4w -B pthreads4w/build -D PT4W_RUN_TESTS=ON
#===============================================================================

# Only on MS Windows we must link to the open source pthreads library.
# Microsoft does not support POSIX threads that we need.
message(CHECK_START "Finding nmake.exe")
    # REQUIRED stops execution with error message
    find_program(NMAKE "nmake.exe" REQUIRED
                 DOC "Native Microsoft make program to build programs from source")
message(CHECK_PASS "found")

if(NOT MSVC)
    message("-- WARNING!")
    message("   You are NOT using the Microsoft (R) C/C++ Optimizing Compiler (MSVC).")
    message("   Using your compiler is not tested and may not work.")
endif()

# Search the compressed archive of the open source POSIX threads library
# for Microsoft Windows that we need to install
file(GLOB PTHREADS4W_ARCHIVE
     LIST_DIRECTORIES false CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/pthreads4w-code*.zip")

# Extract the pthreads library archive
file(ARCHIVE_EXTRACT
     INPUT ${PTHREADS4W_ARCHIVE}
     DESTINATION ${PROJECT_BINARY_DIR})
# Get the directory name of the extracted pthreads archive
file(GLOB PTHREADS4W_BUILD_PATH
     LIST_DIRECTORIES true
    "${PROJECT_BINARY_DIR}/pthreads4w-code*")

# According to the README of pthreads4s we need these header files for compiling
file(COPY ${PTHREADS4W_BUILD_PATH}/_ptw32.h DESTINATION ${PTHREADS4W_BINARY_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/pthread.h DESTINATION ${PTHREADS4W_BINARY_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/sched.h DESTINATION ${PTHREADS4W_BINARY_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/semaphore.h DESTINATION ${PTHREADS4W_BINARY_DIR}/include)

# First we build the static pthreads library with its native installation.
# We call it as an external process.
message(CHECK_START "building pthreads4w static library")
execute_process(COMMAND ${NMAKE} realclean VC-static
                WORKING_DIRECTORY ${PTHREADS4W_BUILD_PATH}
                RESULT_VARIABLE RETURN_CODE
                ERROR_VARIABLE PTHREAD_ERROR_MESSAGE
                COMMAND_ECHO NONE)
if(NOT ${RETURN_CODE} EQUAL 0)
    # This will stop the installation
    message(FATAL_ERROR ${PTHREAD_ERROR_MESSAGE})
endif()

# Save the static pthreads4w library
file(GLOB FILES_TO_COPY ${PTHREADS4W_BUILD_PATH}/libpthreadV*.lib)
file(COPY ${FILES_TO_COPY} DESTINATION ${PTHREADS4W_BINARY_DIR}/lib)
message(CHECK_PASS "done")

message(CHECK_START "building pthreads4w shared library")
# Then we build the shared pthreads library with its native installation.
# We call it as an external process.
execute_process(COMMAND ${NMAKE} realclean VC
                WORKING_DIRECTORY ${PTHREADS4W_BUILD_PATH}
                RESULT_VARIABLE RETURN_CODE
                ERROR_VARIABLE PTHREAD_ERROR_MESSAGE
                COMMAND_ECHO NONE)
if(NOT ${RETURN_CODE} EQUAL 0)
    # This will stop the installation
    message(FATAL_ERROR ${PTHREAD_ERROR_MESSAGE})
endif()

# Save the shared pthreads4w library
file(GLOB FILES_TO_COPY ${PTHREADS4W_BUILD_PATH}/pthreadV*.lib)
file(COPY ${FILES_TO_COPY} DESTINATION ${PTHREADS4W_BINARY_DIR}/lib)
file(GLOB FILES_TO_COPY ${PTHREADS4W_BUILD_PATH}/pthreadV*.dll)
file(COPY ${FILES_TO_COPY} DESTINATION ${PTHREADS4W_BINARY_DIR}/lib)
message(CHECK_PASS "done")

if(PT4W_RUN_TESTS)
message(CHECK_START "run pthreads4w tests")
# Running tests on the pthreads library with executing its native testsuite
# as an external process. This takes some time.
    # commandline options always cached but this one shouldn't
    unset(PT4W_RUN_TESTS CACHE)
    execute_process(COMMAND ${NMAKE} VC
                    WORKING_DIRECTORY ${PTHREADS4W_BUILD_PATH}/tests
                    RESULT_VARIABLE RETURN_CODE
                    COMMAND_ECHO STDOUT)
    if(NOT ${RETURN_CODE} EQUAL 0)
        # This will stop the installation
        message(FATAL_ERROR)
    endif()
message(CHECK_PASS "done")
endif()

IF(NOT UPNP_DEVEL)
    # Clean up build files. We don't need them anymore.
    file(REMOVE_RECURSE ${PTHREADS4W_BUILD_PATH})
endif()
