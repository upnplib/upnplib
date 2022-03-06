# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-03-06

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(PUPNP VERSION 0003
                  DESCRIPTION "Old code from the pupnp library"
                  HOMEPAGE_URL "https://github.com/pupnp")

add_subdirectory(${PROJECT_SOURCE_DIR}/ixml)
add_subdirectory(${PROJECT_SOURCE_DIR}/upnp)
