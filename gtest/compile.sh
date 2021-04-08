#!/usr/bin/bash
SOURCE_DIR="$HOME/devel/upnplib-dev/upnplib"
BUILD_DIR="$HOME/devel/upnplib-dev/upnplib-build"
TESTNAME=$(/usr/bin/basename -s.cpp "$1")
/usr/bin/g++ -std=c++11 -pedantic-errors -Wall \
-o"$TESTNAME".a \
-I"$BUILD_DIR"/googletest-src/googletest/include \
-I"$BUILD_DIR"/googletest-src/googlemock/include \
-I"$SOURCE_DIR" \
-I"$SOURCE_DIR"/upnp/src \
-I"$SOURCE_DIR"/upnp/inc \
-I"$SOURCE_DIR"/upnp/src/inc \
-I"$SOURCE_DIR"/upnp/src/threadutil \
-I"$SOURCE_DIR"/ixml/inc \
-DUPNP_ENABLE_IPV6 \
"$1" \
"$BUILD_DIR"/lib/libgtestd.a \
"$BUILD_DIR"/lib/libgmockd.a \
"$BUILD_DIR"/upnp/libupnp.a \
"$BUILD_DIR"/ixml/libixml.a \
-lpthread
