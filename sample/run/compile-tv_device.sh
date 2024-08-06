#!/usr/bin/bash

SOURCE_DIR="Pupnp/upnp/sample"
EXECUTABLE=$(dirname "$0")"/tv_device-"

if [[ "$1" == "pupnp" ]]; then
    EXECUTABLE=$EXECUTABLE"psh"
elif [[ "$1" == "compa" ]]; then
    EXECUTABLE=$EXECUTABLE"csh"
fi

/usr/bin/g++ -std=c++23 \
-Wall -Wpedantic -Wextra -Werror -Wuninitialized -Wsuggest-override -Wdeprecated \
-o"$EXECUTABLE" \
-DUPNP_HAVE_TOOLS \
-I/usr/local/include/upnp \
-I"$SOURCE_DIR"/common \
"$SOURCE_DIR"/linux/tv_device_main.c \
"$SOURCE_DIR"/common/tv_device.c \
"$SOURCE_DIR"/common/sample_util.c \
-lupnpsdk-$1 \
-lixml

# -DNDEBUG \
# -DHAVE_PTHREAD \
# -DUPNP_HAVE_TOOLS \
# -I/usr/local/include/upnp \
# -I"$SOURCE_DIR"/common \
# "$SOURCE_DIR"/linux/tv_device_main.c \
# "$SOURCE_DIR"/common/tv_device.c \
# "$SOURCE_DIR"/common/sample_util.c \
# -lupnpsdk-pupnp \
# -lupnpsdk-compa \
# -lixml \
# -lpthread

if [ "$?" -ne 0 ]; then
    >&2 echo -e "\nHint: syntax is"
    >&2 echo "$0 [pupnp | compa]"
fi
