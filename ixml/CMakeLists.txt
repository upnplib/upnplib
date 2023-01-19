# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-01-19

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(IXML VERSION 0001
             DESCRIPTION "UPnP library ixml subproject"
             HOMEPAGE_URL "https://github.com/upnplib")


add_compile_options(
    # Most compiler warnings enabled
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    # all compiler warnings as errors
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
)
