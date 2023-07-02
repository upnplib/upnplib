# Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-07-03

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(APPS VERSION 0001
        DESCRIPTION "Applications using different UPnPlib libraries"
        HOMEPAGE_URL "https://github.com/upnplib")


# Info executable linked with the pupnp shared library
# ----------------------------------------------------
add_executable(upnplib-psh
        ./src/upnplib.cpp
)
target_include_directories(upnplib-psh
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
    PRIVATE ${COMPA_SOURCE_DIR}/include
)
target_compile_definitions (upnplib-psh
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNPLIB_SHARED
)
target_link_libraries(upnplib-psh
    PRIVATE
        pupnp_shared
)

# Info executable linked with the pupnp static library
#-----------------------------------------------------
add_executable(upnplib-pst
        ./src/upnplib.cpp
)
target_include_directories(upnplib-pst
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
    PRIVATE ${COMPA_SOURCE_DIR}/include
)
target_compile_definitions (upnplib-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(upnplib-pst
    PRIVATE
        pupnp_static
        # On Linux we cannot link with the static pthreads library
        # because then we have also link with a static libc that's not
        # available by default.
        $<$<NOT:$<PLATFORM_ID:Windows>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
)

# Info executable linked with the compatible shared library
# ---------------------------------------------------------
add_executable(upnplib-csh
        ./src/upnplib.cpp
)
target_include_directories(upnplib-csh
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
)
target_compile_definitions (upnplib-csh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(upnplib-csh
    PRIVATE
        compa_shared
)

# Info executable linked with the compatible static library
#----------------------------------------------------------
add_executable(upnplib-cst
        ./src/upnplib.cpp
)
target_include_directories(upnplib-cst
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
)
target_link_libraries(upnplib-cst
    PRIVATE
        compa_static
        # On Linux we cannot link with the static pthreads library
        # because then we have also link with a static libc that's not
        # available by default.
        $<$<NOT:$<PLATFORM_ID:Windows>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
)
