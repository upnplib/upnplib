# Copyright 2021 GPL 3 or higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-24

# This is included from the main CMakeLists.txt and contains all settings
# to configure and build the project on Microsoft Windows.
#========================================================================

# Search the compressed archive of the open source POSIX threads library
# for Microsoft Windows that we need to install
file(GLOB PTHREADS4W_ARCHIVE
     LIST_DIRECTORIES false CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/pthreads4w-code*.zip")

if(NOT WIN32)

    # If not on MS Windows we don't need the pthreads library
    file(REMOVE ${PTHREADS4W_ARCHIVE})

    return()
endif()

# Only on MS Windows we must link to the open source pthreads library.
# Microsoft does not support POSIX threads that we need.
message(CHECK_START "Finding nmake.exe")
    # REQUIRED stops execution with error message
    find_program(NMAKE "nmake.exe" REQUIRED
                 DOC "Native Microsoft make program to build programs from source")
message(CHECK_PASS "found")

if(NOT MSVC)
    message("-- WARNING!")
    message("   You are NOT using the Microsoft (R) C/C++ Optimizing Compiler.")
    message("   Using your compiler is not tested and may not work.")
endif()

# extract the pthreads library archive
file(ARCHIVE_EXTRACT
     INPUT ${PTHREADS4W_ARCHIVE}
     DESTINATION ${PROJECT_BINARY_DIR})
# Get the directory name of the extracted pthreads archive
file(GLOB PTHREADS4W_BUILD_PATH
     LIST_DIRECTORIES true
    "${PROJECT_BINARY_DIR}/pthreads4w-code*")

# Build the pthreads library
message(CHECK_START "building pthreads4w library")
execute_process(COMMAND ${NMAKE} realclean VC
                WORKING_DIRECTORY ${PTHREADS4W_BUILD_PATH}
                RESULT_VARIABLE RETURN_CODE
                ERROR_VARIABLE PTHREAD_ERROR_MESSAGE
                COMMAND_ECHO NONE)
if(NOT ${RETURN_CODE} EQUAL 0)
    # This will stop the installation
    message(FATAL_ERROR ${PTHREAD_ERROR_MESSAGE})
endif()

if(BUILDTESTS)
# Running tests on the pthreads library
    execute_process(COMMAND ${NMAKE} VC
                    WORKING_DIRECTORY ${PTHREADS4W_BUILD_PATH}/tests
                    RESULT_VARIABLE RETURN_CODE
                    COMMAND_ECHO STDOUT)
    if(NOT ${RETURN_CODE} EQUAL 0)
        # This will stop the installation
        message(FATAL_ERROR)
    endif()
endif()

# Install pthreads binaries and header files for use only local.
# It will not clutter your system.
# Copy following files is normaly done with "nmake install" from the libraries
# installation program but its buggy. Look at its Makefile what
# "nmake install" should do. I have done it here instead.
file(COPY ${PTHREADS4W_BUILD_PATH}/pthread.obj DESTINATION ${PTHREADS4W_DIR}/bin)
file(GLOB FILES_TO_COPY ${PTHREADS4W_BUILD_PATH}/pthreadV*.dll)
file(COPY ${FILES_TO_COPY} DESTINATION ${PTHREADS4W_DIR}/bin)
file(GLOB FILES_TO_COPY ${PTHREADS4W_BUILD_PATH}/pthreadV*.lib)
file(COPY ${FILES_TO_COPY} DESTINATION ${PTHREADS4W_DIR}/lib)
file(COPY ${PTHREADS4W_BUILD_PATH}/_ptw32.h DESTINATION ${PTHREADS4W_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/pthread.h DESTINATION ${PTHREADS4W_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/sched.h DESTINATION ${PTHREADS4W_DIR}/include)
file(COPY ${PTHREADS4W_BUILD_PATH}/semaphore.h DESTINATION ${PTHREADS4W_DIR}/include)

# Clean up build files. We don't need them anymore.
file(REMOVE_RECURSE ${PTHREADS4W_BUILD_PATH})
message(CHECK_PASS "done")


# Import the pthreads library to this build system to be able to link it
add_library(pthreads4w STATIC IMPORTED)
set_target_properties(pthreads4w PROPERTIES IMPORTED_LOCATION
                      ${PTHREADS4W_DIR}/bin/pthread)

# In addition to pthreads we need some more system libraries
# ws2_32: winsock to support sockets
# iphlpapi: ip helper interface to get network management tools
target_link_libraries(upnplib ws2_32 iphlpapi pthreads4w)

# I got warning LNK4098: defaultlib 'MSVCRTD' conflicts with use of other libs;
# use /NODEFAULTLIB:library
target_link_libraries(upnplib -NODEFAULTLIB:MSVCRTD)
