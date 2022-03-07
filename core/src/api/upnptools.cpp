// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-07

#include "upnplib/upnptools.hpp"

#ifndef UPNPLIB_PUPNP_EMULATION
#include "pupnp/upnp/src/api/upnptools.cpp"
#else

#include "upnp.hpp"

const char* UpnpGetErrorMessage(int rc) { return upnplib::errCstr(rc); }

#endif // UPNPLIB_PUPNP_EMULATION

//
namespace upnplib {

struct ErrorString {
    int rc;              // Error code.
    const char* rcError; // Error description.
};

constexpr struct ErrorString ErrorMessages[] = {
    {UPNP_E_SUCCESS, "UPNP_E_SUCCESS"},
    {UPNP_E_INVALID_HANDLE, "UPNP_E_INVALID_HANDLE"},
    {UPNP_E_INVALID_PARAM, "UPNP_E_INVALID_PARAM"},
    {UPNP_E_OUTOF_HANDLE, "UPNP_E_OUTOF_HANDLE"},
    {UPNP_E_OUTOF_CONTEXT, "UPNP_E_OUTOF_CONTEXT"},
    {UPNP_E_OUTOF_MEMORY, "UPNP_E_OUTOF_MEMORY"},
    {UPNP_E_INIT, "UPNP_E_INIT"},
    {UPNP_E_BUFFER_TOO_SMALL, "UPNP_E_BUFFER_TOO_SMALL"},
    {UPNP_E_INVALID_DESC, "UPNP_E_INVALID_DESC"},
    {UPNP_E_INVALID_URL, "UPNP_E_INVALID_URL"},
    {UPNP_E_INVALID_SID, "UPNP_E_INVALID_SID"},
    {UPNP_E_INVALID_DEVICE, "UPNP_E_INVALID_DEVICE"},
    {UPNP_E_INVALID_SERVICE, "UPNP_E_INVALID_SERVICE"},
    {UPNP_E_BAD_RESPONSE, "UPNP_E_BAD_RESPONSE"},
    {UPNP_E_BAD_REQUEST, "UPNP_E_BAD_REQUEST"},
    {UPNP_E_INVALID_ACTION, "UPNP_E_INVALID_ACTION"},
    {UPNP_E_FINISH, "UPNP_E_FINISH"},
    {UPNP_E_INIT_FAILED, "UPNP_E_INIT_FAILED"},
    {UPNP_E_URL_TOO_BIG, "UPNP_E_URL_TOO_BIG"},
    {UPNP_E_BAD_HTTPMSG, "UPNP_E_BAD_HTTPMSG"},
    {UPNP_E_ALREADY_REGISTERED, "UPNP_E_ALREADY_REGISTERED"},
    {UPNP_E_INVALID_INTERFACE, "UPNP_E_INVALID_INTERFACE"},
    {UPNP_E_NETWORK_ERROR, "UPNP_E_NETWORK_ERROR"},
    {UPNP_E_SOCKET_WRITE, "UPNP_E_SOCKET_WRITE"},
    {UPNP_E_SOCKET_READ, "UPNP_E_SOCKET_READ"},
    {UPNP_E_SOCKET_BIND, "UPNP_E_SOCKET_BIND"},
    {UPNP_E_SOCKET_CONNECT, "UPNP_E_SOCKET_CONNECT"},
    {UPNP_E_OUTOF_SOCKET, "UPNP_E_OUTOF_SOCKET"},
    {UPNP_E_LISTEN, "UPNP_E_LISTEN"},
    {UPNP_E_TIMEDOUT, "UPNP_E_TIMEDOUT"},
    {UPNP_E_SOCKET_ERROR, "UPNP_E_SOCKET_ERROR"},
    {UPNP_E_FILE_WRITE_ERROR, "UPNP_E_FILE_WRITE_ERROR"},
    {UPNP_E_CANCELED, "UPNP_E_CANCELED"},
    {UPNP_E_EVENT_PROTOCOL, "UPNP_E_EVENT_PROTOCOL"},
    {UPNP_E_SUBSCRIBE_UNACCEPTED, "UPNP_E_SUBSCRIBE_UNACCEPTED"},
    {UPNP_E_UNSUBSCRIBE_UNACCEPTED, "UPNP_E_UNSUBSCRIBE_UNACCEPTED"},
    {UPNP_E_NOTIFY_UNACCEPTED, "UPNP_E_NOTIFY_UNACCEPTED"},
    {UPNP_E_INVALID_ARGUMENT, "UPNP_E_INVALID_ARGUMENT"},
    {UPNP_E_FILE_NOT_FOUND, "UPNP_E_FILE_NOT_FOUND"},
    {UPNP_E_FILE_READ_ERROR, "UPNP_E_FILE_READ_ERROR"},
    {UPNP_E_EXT_NOT_XML, "UPNP_E_EXT_NOT_XML"},
    {UPNP_E_NO_WEB_SERVER, "UPNP_E_NO_WEB_SERVER"},
    {UPNP_E_OUTOF_BOUNDS, "UPNP_E_OUTOF_BOUNDS"},
    {UPNP_E_NOT_FOUND, "UPNP_E_NOT_FOUND"},
    {UPNP_E_INTERNAL_ERROR, "UPNP_E_INTERNAL_ERROR"},
};

const char* errCstr(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return ErrorMessages[i].rcError;
        }
    }
    return "Unknown error code";
}

const std::string errStrEx(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return (std::string)ErrorMessages[i].rcError + "(" +
                   std::to_string(rc) + ")";
        }
    }
    return "Unknown error code(" + std::to_string(rc) + ")";
}

} // namespace upnplib
