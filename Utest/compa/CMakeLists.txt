# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-03-22

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0011
                  DESCRIPTION "Tests for pupnp and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


add_subdirectory(0-addressing)
add_subdirectory(1-discovery)
add_subdirectory(api.d)
add_subdirectory(http.d)
add_subdirectory(threadutil.d)
add_subdirectory(uri.d)
add_subdirectory(util.d)
