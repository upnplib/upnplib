# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-05-19

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNPLIB_GTESTS VERSION 0024
                       DESCRIPTION "Unit Tests using googletest"
                       HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################
# For old builds with direct build from source files look at git commit:
# git show 741a85fbc88:gtests/CMakeLists.txt

# To link googletest we need of course its libraries.
# gmock libraries also contain gtest libraries.
link_libraries(gmock gmock_main
)

# Reset output directories to default means to use the subprojects directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "")

# If linking with shared build gtest libs we need to tell it the compiler.
# I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
# build/_deps/googletest-src/googletest/README.md
add_compile_definitions(
    PRIVATE $<$<AND:$<BOOL:${UPNPLIB_WITH_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
    # Compile tests for the old (pupnp) source code.
    OLD_TEST
)

# Create upnplib helper tools library for gtests. We have version
# upnplib_gtest_tools for all operating systems,
# upnplib_gtest_tools_unix for Unix operating systems and
# upnplib_gtest_tools_win32 for Microsoft Windows
add_library(upnplib_gtest_tools SHARED
        ./tools/upnplib_gtest_tools.cpp
        $<$<BOOL:${WIN32}>:./tools/upnplib_gtest_tools_win32.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:./tools/upnplib_gtest_tools_unix.cpp>
)
target_include_directories(upnplib_gtest_tools
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}/include
)
target_compile_definitions(upnplib_gtest_tools
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
)
target_link_libraries(upnplib_gtest_tools
    PUBLIC
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
    INTERFACE
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
)

#################################
# Unit Tests                    #
#################################
# The tests are build in reverse order. test_template.cpp will be build first.

# sock
#-----
add_executable(test_sock
        ./test_sock.cpp
)
target_include_directories(test_sock
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_BINARY_DIR}/inc
)
target_compile_definitions(test_sock PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_sock
    PRIVATE
        upnplib_compa_shared
        upnplib_native_shared
)
add_test(NAME ctest_sock COMMAND test_sock
)

# strintmap
#----------
add_executable(test_strintmap
    ./test_strintmap.cpp
)
target_include_directories(test_strintmap
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_strintmap PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_strintmap
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_strintmap COMMAND test_strintmap
)

# list
#-----
add_executable(test_list
        ./test_list.cpp
)
target_include_directories(test_list
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
)
add_test(NAME ctest_list COMMAND test_list
)

# membuffer
#----------
add_executable(test_membuffer
        ./test_membuffer.cpp
)
target_include_directories(test_membuffer
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
)
add_test(NAME ctest_membuffer COMMAND test_membuffer
)

# uri
#----
# To test static functions we cannot link with libraries. So we build direct
# with the source files.
add_executable(test_uri
	${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
	${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
	./test_uri.cpp
)
target_include_directories(test_uri
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_BINARY_DIR}/inc
)
target_link_libraries(test_uri
    PRIVATE
        upnplib_gtest_tools
        upnplib_native_shared
)
add_test(NAME ctest_uri COMMAND test_uri)

if(FALSE)
# Link with internal libraries.
add_executable(test_uri
    ./test_uri.cpp
)
target_include_directories(test_uri
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_uri PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_uri
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_uri COMMAND test_uri
)
endif()

# uri: uri_parse
#---------------
# To test static functions we cannot link with libraries. So we build direct
# with the source files.
add_executable(test_uri_parse
        ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
        ./test_uri.d/test_uri_parse.cpp
)
target_include_directories(test_uri_parse
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_BINARY_DIR}/inc
)
target_link_libraries(test_uri_parse
    PRIVATE
        upnplib_native_shared
)
add_test(NAME ctest_uri_parse COMMAND test_uri_parse)

if(FALSE)
# Link with internal libraries.
add_executable(test_uri_parse
    ./test_uri.d/test_uri_parse.cpp
)
target_include_directories(test_uri_parse
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_uri_parse PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_uri_parse
        PRIVATE upnplib_compa_shared
        PRIVATE upnplib_native_shared
)
add_test(NAME ctest_uri_parse COMMAND test_uri_parse
)
endif()

# uri: urlparser
#---------------
add_executable(test_urlparser
    ./test_uri.d/test_urlparser.cpp
)
target_include_directories(test_urlparser
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
)
add_test(NAME ctest_urlparser COMMAND test_urlparser
)

# uri: url_class
#---------------
add_executable(test_url_class
    ./test_uri.d/test_url_class.cpp
)
target_include_directories(test_url_class
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
)
add_test(NAME ctest_url_class COMMAND test_url_class
)

# httpparser
#-----------
if(FALSE)
# Temporary disabled. Include files and directories must be checked.

# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpparser
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ./test_httpparser.cpp
)
#endif()

add_executable(test_httpparser
    ./test_httpparser.cpp
)
target_include_directories(test_httpparser
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpparser PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_httpparser
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_httpparser COMMAND test_httpparser
)
endif()

# httpreadwrite incl. statcodes
#------------------------------
if(FALSE)
# Temporary disabled. Include files and directories must be checked.

# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpreadwrite
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    ./test_httpreadwrite.cpp
)
#endif()

add_executable(test_httpreadwrite
        ./test_httpreadwrite.cpp
)
target_include_directories(test_httpreadwrite
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpreadwrite PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_httpreadwrite
        PRIVATE upnplib_compa_shared
        PRIVATE upnplib_native_shared
        PRIVATE $<IF:$<BOOL:${WIN32}>,ixml_static,ixml_shared>
)
add_test(NAME ctest_httpreadwrite COMMAND test_httpreadwrite
)
endif()

# httpreadwrite: netconnect
#--------------------------
if(FALSE)
# Temporary disabled. Include files and directories must be checked.

# Direct linking with source files currently not used but having available for
# reference. We link against the upnplib libraries (see below).
add_executable(test_netconnect
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    test_httpreadwrite.d/test_netconnect.cpp
)
#endif()

add_executable(test_netconnect
        test_httpreadwrite.d/test_netconnect.cpp
)
target_include_directories(test_netconnect
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_netconnect PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_netconnect
        upnplib_compa_shared
)
add_test(NAME ctest_netconnect COMMAND test_netconnect
)
endif()

# UpnpString
#-----------
add_executable(test_UpnpString
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
)
add_test(NAME ctest_UpnpString COMMAND test_UpnpString)

# upnpdebug
#----------
add_executable(test_upnpdebug
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_upnpdebug PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_upnpdebug
        PRIVATE upnplib_compa_shared
        PRIVATE upnplib_native_shared
        PRIVATE upnplib_gtest_tools
)
add_test(NAME ctest_upnpdebug COMMAND test_upnpdebug)


# upnpapi
#--------
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the upnplib libraries (see below).
set(UPNP_UPNPAPI_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/document.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/element.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/node.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.cpp
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_unix.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
)
endif()

add_executable(test_upnpapi
        $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/test_upnpapi_win32.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:${PROJECT_SOURCE_DIR}/test_upnpapi_unix.cpp>
)
target_include_directories(test_upnpapi
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
)
target_compile_definitions(test_upnpapi PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_upnpapi
        PRIVATE upnplib_gtest_tools
        PRIVATE upnplib_compa_shared
        PRIVATE upnplib_native_shared
)
add_test(NAME ctest_upnpapi COMMAND test_upnpapi
)

if(FALSE)
# LinkedList
#-----------
# Compile direct with sources because including source file under test into
# gtest to also test functions with file scope.
add_executable(test_LinkedList
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ./test_LinkedList.cpp)
add_test(NAME ctest_LinkedList COMMAND test_LinkedList
)
endif()

# LinkedList
#-----------
add_executable(test_LinkedList
        ./test_LinkedList.cpp
)
target_include_directories(test_LinkedList
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_LinkedList PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_LinkedList
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_LinkedList COMMAND test_LinkedList
)

# FreeList
#---------
add_executable(test_FreeList
        ./test_FreeList.cpp
)
target_include_directories(test_FreeList
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_FreeList
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_FreeList COMMAND test_FreeList
)

# ThreadPool
#-----------
add_executable(test_ThreadPool
        ./test_ThreadPool.cpp
)
target_include_directories(test_ThreadPool
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_ThreadPool PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_ThreadPool
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_ThreadPool COMMAND test_ThreadPool
)

# TimerThread
#------------
add_executable(test_TimerThread
        ./test_TimerThread.cpp
)
target_include_directories(test_TimerThread
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        PRIVATE ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_TimerThread PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_TimerThread
        PRIVATE upnplib_compa_shared
)
add_test(NAME ctest_TimerThread COMMAND test_TimerThread
)

# test upnplib_gtest_tools
#-------------------------
add_executable(test_upnplib_gtest_tools
        $<$<BOOL:${WIN32}>:./tools/test_tools_win32.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:./tools/test_tools_unix.cpp>
)
target_include_directories(test_upnplib_gtest_tools
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
)
target_compile_definitions(test_upnplib_gtest_tools
        PRIVATE UPNPLIB_SHARED
)
target_link_libraries(test_upnplib_gtest_tools
        PRIVATE upnplib_gtest_tools
)
add_test(NAME ctest_upnplib_gtest_tools COMMAND test_upnplib_gtest_tools
)

# upnptools
#----------
add_executable(test_upnptools
        test_upnptools.cpp
)
target_include_directories(test_upnptools
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_link_libraries(test_upnptools
        PRIVATE pupnp_native_static
        PRIVATE upnplib_native_static
)
add_test(NAME ctest_upnptools COMMAND test_upnptools
)

add_executable(test_upnptools_compa
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools_compa
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
        PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
        PRIVATE ${PUPNP_BINARY_DIR}/inc
        PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_link_libraries(test_upnptools_compa
        PRIVATE upnplib_compa_static
        PRIVATE upnplib_native_static
)
add_test(NAME ctest_upnptools_compa COMMAND test_upnptools_compa
)

# simple test
#------------
add_executable(test_simple
        ./test_simple.cpp
)
target_include_directories(test_simple
    PRIVATE
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_link_libraries(test_simple
    PRIVATE
        upnplib_compa_static
)
add_test(NAME ctest_simple COMMAND test_simple)

# template
#---------
add_executable(test_template
        test_template.cpp)
target_include_directories(test_template
        PRIVATE ./include)
add_test(NAME ctest_template COMMAND test_template)


#add_subdirectory(${PROJECT_SOURCE_DIR}/test_sample.d)

# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
