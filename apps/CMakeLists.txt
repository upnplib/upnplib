# Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-07-16

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(APPS VERSION 0002
        DESCRIPTION "Applications using different UPnPlib libraries"
        HOMEPAGE_URL "https://github.com/upnplib")


# Info executable linked with the pupnp shared library
# ----------------------------------------------------
add_executable(upnplib-psh
        ./src/upnplib.cpp
)
target_link_libraries(upnplib-psh
    PRIVATE pupnp_shared
)

# Info executable linked with the pupnp static library
#-----------------------------------------------------
add_executable(upnplib-pst
        ./src/upnplib.cpp
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
target_link_libraries(upnplib-csh
    PRIVATE
        compa_shared
)

# Info executable linked with the compatible static library
#----------------------------------------------------------
add_executable(upnplib-cst
        ./src/upnplib.cpp
)
target_link_libraries(upnplib-cst
    PRIVATE
        compa_static
        # On Linux we cannot link with the static pthreads library
        # because then we have also link with a static libc that's not
        # available by default.
        $<$<NOT:$<PLATFORM_ID:Windows>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
)
