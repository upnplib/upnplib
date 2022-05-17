// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-17

// Here we only define global variables. There are no other references so we do
// not have to compile additional sources only to access a global.

#include <cstddef>
#include "config.hpp"
#include "UpnpGlobal.hpp" // for EXPORT_SPEC

/*! Contains interface index. */
EXPORT_SPEC unsigned int gIF_INDEX = (unsigned)-1;

/*! Maximum content-length (in bytes) that the SDK will process on an incoming
 * packet. Content-Length exceeding this size will be not processed and
 * error 413 (HTTP Error Code) will be returned to the remote end point. */
EXPORT_SPEC size_t g_maxContentLength = DEFAULT_SOAP_CONTENT_LENGTH;
