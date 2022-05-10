// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-10

#include "pupnp/upnp/src/api/upnptools.cpp"

const char* UpnpGetErrorMessage(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return ErrorMessages[i].rcError;
        }
    }
    return "Unknown error code";
}
