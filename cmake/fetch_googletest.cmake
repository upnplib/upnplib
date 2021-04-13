# Download, configure and build googletest
#-----------------------------------------
# Author: 2021-04-13 - Ingo Höft, last modified: 2021-04-13

cmake_minimum_required(VERSION 3.14)

Find_Package(Git)
cmake_dependent_option (GOOGLETEST "Download and build Googletest" OFF ${Git_FOUND} OFF)

if (GOOGLETEST)
    include (FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        GIT_TAG           origin/master
        GIT_SHALLOW       ON
        GIT_CONFIG        advice.detachedHead=false
    )

    FetchContent_MakeAvailable(googletest)
endif()
