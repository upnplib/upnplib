# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# last modified: 2021-05-07

#!/usr/bin/bash
# Manual test compile for development, not for production use!

# build with:
#rm -rf ./build/ && cmake -B ./build/ -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DUPNP_GOOGLETEST=OFF -Dclient=OFF -Ddevice=OFF -Dgena=OFF -Dipv6=OFF -Doptssdp=OFF -Dsamples=OFF -Dscriptsupport=OFF -Dsoap=OFF -Dssdp=OFF -Dtools=OFF -LH .

SOURCE_DIR="$HOME/devel/upnplib-dev/upnplib"
BUILD_DIR="$HOME/devel/upnplib-dev/upnplib/build"

#/usr/bin/g++ -std=c++17 -pedantic-errors -Wall -fdiagnostics-color=always \
/usr/bin/gcc -pedantic-errors -Wall -fdiagnostics-color=always \
-oupnplib.a \
-I"$SOURCE_DIR"/upnp/inc \
-I"$SOURCE_DIR"/upnp/src/inc \
-I"$SOURCE_DIR"/upnp/src/threadutil \
-I"$SOURCE_DIR"/ixml/inc \
-I"$SOURCE_DIR"/ixml/src/inc \
-I"$BUILD_DIR" \
-I"$BUILD_DIR"/upnp/inc \
"$SOURCE_DIR"/upnp/src/threadutil/ThreadPool.cpp \
"$SOURCE_DIR"/upnp/src/threadutil/LinkedList.cpp \
"$SOURCE_DIR"/upnp/src/threadutil/TimerThread.cpp \
"$SOURCE_DIR"/upnp/src/threadutil/FreeList.cpp \
"$SOURCE_DIR"/upnp/src/genlib/net/sock.c \
"$SOURCE_DIR"/upnp/src/genlib/net/http/webserver.c \
"$SOURCE_DIR"/upnp/src/genlib/net/http/httpparser.c \
"$SOURCE_DIR"/upnp/src/genlib/net/http/statcodes.c \
"$SOURCE_DIR"/upnp/src/genlib/net/http/httpreadwrite.c \
"$SOURCE_DIR"/upnp/src/genlib/net/uri/uri.c \
"$SOURCE_DIR"/upnp/src/genlib/util/strintmap.c \
"$SOURCE_DIR"/upnp/src/genlib/util/membuffer.c \
"$SOURCE_DIR"/upnp/src/genlib/util/list.c \
"$SOURCE_DIR"/upnp/src/genlib/miniserver/miniserver.c \
"$SOURCE_DIR"/upnp/src/api/UpnpExtraHeaders.c \
"$SOURCE_DIR"/upnp/src/api/UpnpString.c \
"$SOURCE_DIR"/upnp/src/api/UpnpFileInfo.c \
"$SOURCE_DIR"/upnp/src/api/upnpdebug.cpp \
"$SOURCE_DIR"/upnp/src/api/upnpapi.cpp \
"$SOURCE_DIR"/ixml/src/ixml.c \
"$SOURCE_DIR"/ixml/src/ixmlmembuf.c \
"$SOURCE_DIR"/ixml/src/node.c \
"$SOURCE_DIR"/ixml/src/element.c \
"$SOURCE_DIR"/ixml/src/ixmlparser.c \
"$SOURCE_DIR"/ixml/src/attr.c \
"$SOURCE_DIR"/ixml/src/nodeList.c \
"$SOURCE_DIR"/ixml/src/document.c \
"$SOURCE_DIR"/ixml/src/namedNodeMap.c \
\
"$SOURCE_DIR"/upnp/test/test_init.c \
-lpthread


# ssdp
#"$SOURCE_DIR"/upnp/src/ssdp/SSDPResultDataCallback.c \
#"$SOURCE_DIR"/upnp/src/ssdp/SSDPResultData.c \
#"$SOURCE_DIR"/upnp/src/ssdp/ssdp_device.c \
#"$SOURCE_DIR"/upnp/src/ssdp/ssdp_ctrlpt.c \
#"$SOURCE_DIR"/upnp/src/ssdp/ssdp_server.c \

# soap
#"$SOURCE_DIR"/upnp/src/soap/soap_device.c \
#"$SOURCE_DIR"/upnp/src/soap/soap_ctrlpt.c \
#"$SOURCE_DIR"/upnp/src/soap/soap_common.c \

# gena
#"$SOURCE_DIR"/upnp/src/gena/gena_device.c \
#"$SOURCE_DIR"/upnp/src/gena/gena_ctrlpt.c \
#"$SOURCE_DIR"/upnp/src/gena/gena_callback2.c \

# tools
#"$SOURCE_DIR"/upnp/src/api/upnptools.c \

# uuid
#"$SOURCE_DIR"/upnp/src/uuid/md5.c \
#"$SOURCE_DIR"/upnp/src/uuid/sysdep.c \
#"$SOURCE_DIR"/upnp/src/uuid/uuid.c \

# pupnp
#"$SOURCE_DIR"/upnp/src/api/UpnpEventSubscribe.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpActionComplete.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpStateVarComplete.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpEvent.c \
#"$SOURCE_DIR"/upnp/src/genlib/client_table/GenlibClientSubscription.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpActionRequest.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpStateVarRequest.c \
#"$SOURCE_DIR"/upnp/src/genlib/client_table/client_table.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpSubscriptionRequest.c \
#"$SOURCE_DIR"/upnp/src/genlib/service_table/service_table.c \
#"$SOURCE_DIR"/upnp/src/genlib/util/upnp_timeout.c \
#"$SOURCE_DIR"/upnp/src/api/UpnpDiscovery.c \
#"$SOURCE_DIR"/upnp/src/urlconfig/urlconfig.c \
#"$SOURCE_DIR"/upnp/src/genlib/net/http/parsetools.c \
#"$SOURCE_DIR"/upnp/src/genlib/util/util.c \
