#!/usr/bin/bash

if [ "$#" -ne 1 ]; then
    >&2 echo "Compile with: $0 test_tools.cpp"
    >&2 echo 'Set BUILD_DIR to point to the pupnp build directory'
fi

SOURCE_DIR="$HOME/devel/upnplib-dev/upnplib"
BUILD_DIR="$HOME/devel/upnplib-dev/upnplib-build"

TESTNAME=$(/usr/bin/basename -s.cpp "$1")
/usr/bin/g++ -std=c++17 -pedantic-errors -Wall -fdiagnostics-color=always \
-o"$TESTNAME".a \
-I"$BUILD_DIR"/_deps/googletest-src/googletest/include \
-I"$BUILD_DIR"/_deps/googletest-src/googlemock/include \
-I"$BUILD_DIR" \
-I"$BUILD_DIR"/upnp/inc \
-I"$SOURCE_DIR"/upnp/src \
-I"$SOURCE_DIR"/upnp/inc \
-I"$SOURCE_DIR"/upnp/src/inc \
-I"$SOURCE_DIR"/upnp/src/threadutil \
-I"$SOURCE_DIR"/ixml/inc \
-DUPNP_ENABLE_IPV6 \
"$1" \
"$BUILD_DIR"/lib/libgtest.a \
"$BUILD_DIR"/lib/libgmock.a \
"$BUILD_DIR"/upnp/libupnp.a \
"$BUILD_DIR"/ixml/libixml.a \
-lpthread
