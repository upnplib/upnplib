# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-06-29

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(SAMPLE VERSION 0009
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


include_directories(
    ./common
)

add_compile_options(
    # Default warning level
    $<$<CXX_COMPILER_ID:MSVC>:/W1>
    # all compiler warnings as errors
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
)


IF (UPNPLIB_DEF_SAMPLE_CLIENT)
    set(TV_CTRLPT_SOURCE_FILES
        ./common/sample_util.cpp
        ./common/tv_ctrlpt.cpp
        ./linux/tv_ctrlpt_main.cpp
    )

    add_executable (tv_ctrlpt-psh
        ${TV_CTRLPT_SOURCE_FILES}
    )
    target_compile_definitions(tv_ctrlpt-psh
        PRIVATE UPNPLIB_SHARED
        PRIVATE UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(tv_ctrlpt-psh
        PRIVATE pupnp_shared
    )

    add_executable (tv_ctrlpt-csh
        ${TV_CTRLPT_SOURCE_FILES}
    )
    target_compile_definitions(tv_ctrlpt-csh
        PRIVATE UPNPLIB_SHARED
    )
    target_link_libraries(tv_ctrlpt-csh
        PRIVATE compa_shared
    )
ENDIF()

# client and device
IF (UPNPLIB_DEF_SAMPLE_COMBO)
    set(TV_COMBO_SOURCE_FILES
        ./common/sample_util.cpp
        ./common/tv_ctrlpt.cpp
        ./common/tv_device.cpp
        ./linux/tv_combo_main.cpp
    )

    add_executable (tv_combo-psh
        ${TV_COMBO_SOURCE_FILES}
    )
    target_compile_definitions(tv_combo-psh
        PRIVATE UPNPLIB_SHARED
        PRIVATE UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(tv_combo-psh
        PRIVATE pupnp_shared
    )

    add_executable (tv_combo-csh
        ${TV_COMBO_SOURCE_FILES}
    )
    target_compile_definitions(tv_combo-csh
        PRIVATE UPNPLIB_SHARED
    )
    target_link_libraries(tv_combo-csh
        PRIVATE compa_shared
    )
ENDIF()

IF(UPNPLIB_DEF_SAMPLE_DEVICE)
    set(TV_DEVICE_SOURCE_FILES
        ./common/sample_util.cpp
        ./common/tv_device.cpp
        ./linux/tv_device_main.cpp
    )

    add_executable (tv_device-psh
        ${TV_DEVICE_SOURCE_FILES}
    )
    target_compile_definitions(tv_device-psh
        PRIVATE UPNPLIB_SHARED
        PRIVATE UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(tv_device-psh
        PRIVATE pupnp_shared
    )

    add_executable (tv_device-csh
        ${TV_DEVICE_SOURCE_FILES}
    )
    target_compile_definitions(tv_device-csh
        PRIVATE UPNPLIB_SHARED
    )
    target_link_libraries(tv_device-csh
        PRIVATE compa_shared
    )
ENDIF()
