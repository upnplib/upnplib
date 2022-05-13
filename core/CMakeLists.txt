# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-05-13

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_CORE VERSION 0004
                     DESCRIPTION "UPnP library core program"
                     HOMEPAGE_URL "https://github.com/upnplib")


include_directories(
    ${UPnPlib_BINARY_DIR}/upnp/inc
    ${PUPNP_UPNP_SOURCE_DIR}/inc
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
    ${PUPNP_IXML_SOURCE_DIR}/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}
)

if(NOT WIN32)

    # upnplib compatible shared library
    #----------------------------------
    add_library(upnplib_compa_shared SHARED
            ./src/api/upnptools.cpp
    )
    set_target_properties(upnplib_compa_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_shared
            PRIVATE ${UPnPlib_SOURCE_DIR}
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
    )
    target_compile_definitions(upnplib_compa_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_EXPORTS
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_shared
            PUBLIC pupnp_native_shared
    )

    # upnplib compatible static library
    #----------------------------------
    add_library(upnplib_compa_static STATIC
            ./src/api/upnptools.cpp
    )
    set_target_properties(upnplib_compa_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_static
            PRIVATE ${UPnPlib_SOURCE_DIR}
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_static
            PUBLIC pupnp_native_static
    )

    # upnplib native shared library
    #------------------------------
    add_library(upnplib_native_shared SHARED
            ./src/api/upnplib/upnptools.cpp
    )
    set_target_properties(upnplib_native_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_shared
            PRIVATE ${UPnPlib_SOURCE_DIR}/include
    )
    target_compile_definitions(upnplib_native_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_EXPORTS
    )

    # upnplib native static library
    #------------------------------
    add_library(upnplib_native_static STATIC
            ./src/api/upnplib/upnptools.cpp
    )
    set_target_properties(upnplib_native_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_static
            PRIVATE ${UPnPlib_SOURCE_DIR}/include
    )

    # Info executable linked with the pupnp native shared library
    #------------------------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_compile_definitions (upnplibInfo_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_options(upnplibInfo_shared
        PRIVATE $<$<BOOL:${APPLE}>:LINKER:-rpath,${UPnPlib_BINARY_DIR}/core/lib,-rpath,${UPnPlib_BINARY_DIR}/pupnp/ixml>
    )
    target_link_libraries(upnplibInfo_shared
            PRIVATE pupnp_native_shared
            PRIVATE ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the upnplib compatible static library
    #------------------------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_link_libraries(upnplibInfo_static
            PRIVATE upnplib_compa_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            PRIVATE ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # upnplib compatible shared library
    #----------------------------------
    add_library(upnplib_compa_shared SHARED
            ./src/api/upnptools.cpp
    )
    set_target_properties(upnplib_compa_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_include_directories(upnplib_compa_shared
            PRIVATE ${UPnPlib_SOURCE_DIR}
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
    )
    target_compile_definitions(upnplib_compa_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_EXPORTS
            PRIVATE $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_shared
            PUBLIC pupnp_native_shared
    )

    # upnplib compatible static library
    #----------------------------------
    add_library(upnplib_compa_static STATIC
            ./src/api/upnptools.cpp
    )
    set_target_properties(upnplib_compa_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_include_directories(upnplib_compa_static
            PRIVATE ${UPnPlib_SOURCE_DIR}
            PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
    )
    target_compile_definitions(upnplib_compa_static
            PRIVATE $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_static
            PUBLIC pupnp_native_static
    )

    # upnplib native shared library
    #------------------------------
    add_library(upnplib_native_shared SHARED
            ./src/api/upnplib/upnptools.cpp
    )
    set_target_properties(upnplib_native_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_include_directories(upnplib_native_shared
            PRIVATE ${UPnPlib_SOURCE_DIR}/include
    )
    target_compile_definitions(upnplib_native_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_EXPORTS
            PRIVATE $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # In addition to pthreads we need some more system libraries here
    target_link_libraries(upnplib_native_shared
            #PRIVATE ws2_32   # winsock to support sockets
            #PRIVATE iphlpapi # ip helper interface to get network management tools
            #PUBLIC ixml_shared
            PUBLIC ${UPNP_PTHREADS_STATIC_LIBRARY}
            # If you link with the shared pthreads4w library then its .dll file
            # must also be found.
            #${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # upnplib native static library
    #------------------------------
    add_library(upnplib_native_static STATIC
            ./src/api/upnplib/upnptools.cpp
    )
    set_target_properties(upnplib_native_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_include_directories(upnplib_native_static
            PRIVATE ${UPnPlib_SOURCE_DIR}/include
    )
    target_compile_definitions(upnplib_native_static
            PRIVATE $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )

    # Info executable linked with the pupnp native shared library
    #------------------------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_compile_definitions (upnplibInfo_shared
            PRIVATE UPNPLIB_SHARED
            PRIVATE UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(upnplibInfo_shared
            PRIVATE pupnp_native_shared
            PRIVATE ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the upnplib compatible static library
    #------------------------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
    )
    target_link_libraries(upnplibInfo_static
            PRIVATE ws2_32   # winsock to support sockets
            PRIVATE ixml_static
            PRIVATE upnplib_compa_static
    )

endif() # WIN32
