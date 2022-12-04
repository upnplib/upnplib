# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-04

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_SAMPLE VERSION 0008
                       DESCRIPTION "Simple sample to use the library"
                       HOMEPAGE_URL "https://github.com/upnplib")


# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
# To build all samples by default then use:
# cmake -S . -B build -D UPNPLIB_WITH_SAMPLES=YES
# To build only a single sample for testing and internal use you can do e.g.:
# cmake -S . -B build -D UPNPLIB_DEF_SAMPLES=YES -D UPNPLIB_DEF_SAMPLE_CLIENT=YES
if(UPNPLIB_WITH_SAMPLES)
    set(UPNPLIB_DEF_SAMPLE_DEVICE YES)
    set(UPNPLIB_DEF_SAMPLE_CLIENT YES)
    set(UPNPLIB_DEF_SAMPLE_COMBO YES)
endif()


set(SAMPLE_INCLUDE_DIRECTORIES
    PRIVATE
        ./common
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)


link_libraries(
        pupnp_shared
        upnplib_shared
)

IF (UPNPLIB_DEF_SAMPLE_CLIENT)
    add_executable (tv_ctrlpt
        ./common/sample_util.cpp
        ./common/tv_ctrlpt.cpp
        ./linux/tv_ctrlpt_main.cpp
    )
    target_include_directories(tv_ctrlpt
        ${SAMPLE_INCLUDE_DIRECTORIES}
    )
ENDIF()

# client and device
IF (UPNPLIB_DEF_SAMPLE_COMBO)
    add_executable (tv_combo
        ./common/sample_util.cpp
        ./common/tv_ctrlpt.cpp
        ./common/tv_device.cpp
        ./linux/tv_combo_main.cpp
    )
    target_include_directories(tv_combo
        ${SAMPLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(tv_combo
        PRIVATE
            # This is to avoid to compile in 'upnpdebug' that has many
            # dependencies into the libary and would pull many other sources
            NDEBUG
    )

ENDIF()

IF(UPNPLIB_DEF_SAMPLE_DEVICE)
    add_executable (tv_device
            ./common/sample_util.cpp
            ./common/tv_device.cpp
            ./linux/tv_device_main.cpp
    )
    target_include_directories(tv_device
        ${SAMPLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(tv_device
        PRIVATE
            # This is to avoid to compile in 'upnpdebug' that has many
            # dependencies into the libary and would pull many other sources
            NDEBUG
    )
ENDIF()
