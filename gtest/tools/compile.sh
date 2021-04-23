#!/usr/bin/bash

if [ "$#" -ne 1 ]; then
    >&2 echo "Compile with: $0 test_tools.cpp"
    >&2 echo 'Set BUILD_DIR to point to the pupnp build directory'
fi

SOURCE_DIR="$HOME/devel/upnplib-dev/upnplib/gtest"

#BUILD_DIR="$HOME/devel/upnplib-dev/upnplib/build"

TESTNAME=$(/usr/bin/basename -s.cpp "$1")
/usr/bin/g++ -std=c++17 -pedantic-errors -Wall -fdiagnostics-color=always \
-o"$TESTNAME".a \
-I"$SOURCE_DIR"/inc \
-I"$SOURCE_DIR"/../build/_deps/googletest-src/googletest/include \
-I"$SOURCE_DIR"/../build/_deps/googletest-src/googlemock/include \
-I"$SOURCE_DIR"/../build \
-I"$SOURCE_DIR"/../upnp/inc \
-I"$SOURCE_DIR"/../upnp/src \
-I"$SOURCE_DIR"/../upnp/src/inc \
-I"$SOURCE_DIR"/../upnp/src/threadutil \
-I"$SOURCE_DIR"/../build/upnp/inc \
-I"$SOURCE_DIR"/../ixml/inc \
"$1" \
"$SOURCE_DIR"/../build/lib/libgtest.a \
"$SOURCE_DIR"/../build/lib/libgmock.a \
"$SOURCE_DIR"/../build/upnp/libupnp.a \
"$SOURCE_DIR"/../build/ixml/libixml.a \
-lpthread

#-DUPNP_ENABLE_IPV6 \
