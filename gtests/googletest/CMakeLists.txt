cmake_minimum_required(VERSION 3.18)
project(UPNP_GTEST_DOWNLOAD)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           origin/master
    GIT_SHALLOW       ON
    GIT_CONFIG        advice.detachedHead=false
)
FetchContent_MakeAvailable(googletest)
