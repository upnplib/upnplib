// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-25
/*!
 * \file
 * \brief General usable free function tools and helper
 */

#include <upnplib/upnptools.hpp>
#include <upnplib/messages.hpp>

namespace {

/*!
 * \brief Sample: assign {UPNP_E_SUCCESS, "UPNP_E_SUCCESS"}
 *
 * Table with assignment of the integer message number to its text string.
 */
struct ErrorString {
    int rc; ///< Error code.
    const char* rcError; ///< Error description.
};

/*!
 * \brief Array with text names of error messages.
 *
 * This const array is initialized with the character string names of the
 * UPNP_E_* error messages so we can translate the integer number of a message
 * to its human readable name for output.
 */
constexpr ErrorString ErrorMessages[] = {
    {UPNP_E_SUCCESS, "UPNP_E_SUCCESS"},
    {UPNP_E_INVALID_HANDLE, "UPNP_E_INVALID_HANDLE"},
    {UPNP_E_INVALID_PARAM, "UPNP_E_INVALID_PARAM"},
    {UPNP_E_OUTOF_HANDLE, "UPNP_E_OUTOF_HANDLE"},
    // {UPNP_E_OUTOF_CONTEXT, "UPNP_E_OUTOF_CONTEXT"},
    {UPNP_E_OUTOF_MEMORY, "UPNP_E_OUTOF_MEMORY"},
    {UPNP_E_INIT, "UPNP_E_INIT"},
    // {UPNP_E_BUFFER_TOO_SMALL, "UPNP_E_BUFFER_TOO_SMALL"},
    {UPNP_E_INVALID_DESC, "UPNP_E_INVALID_DESC"},
    {UPNP_E_INVALID_URL, "UPNP_E_INVALID_URL"},
    // {UPNP_E_INVALID_SID, "UPNP_E_INVALID_SID"},
    // {UPNP_E_INVALID_DEVICE, "UPNP_E_INVALID_DEVICE"},
    {UPNP_E_INVALID_SERVICE, "UPNP_E_INVALID_SERVICE"},
    {UPNP_E_BAD_RESPONSE, "UPNP_E_BAD_RESPONSE"},
    // {UPNP_E_BAD_REQUEST, "UPNP_E_BAD_REQUEST"},
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
    // {UPNP_E_FILE_WRITE_ERROR, "UPNP_E_FILE_WRITE_ERROR"},
    {UPNP_E_CANCELED, "UPNP_E_CANCELED"},
    {UPNP_E_SOCKET_ACCEPT, "UPNP_E_SOCKET_ACCEPT"},
    // {UPNP_E_EVENT_PROTOCOL, "UPNP_E_EVENT_PROTOCOL"},
    {UPNP_E_SUBSCRIBE_UNACCEPTED, "UPNP_E_SUBSCRIBE_UNACCEPTED"},
    {UPNP_E_UNSUBSCRIBE_UNACCEPTED, "UPNP_E_UNSUBSCRIBE_UNACCEPTED"},
    {UPNP_E_NOTIFY_UNACCEPTED, "UPNP_E_NOTIFY_UNACCEPTED"},
    {UPNP_E_INVALID_ARGUMENT, "UPNP_E_INVALID_ARGUMENT"},
    {UPNP_E_FILE_NOT_FOUND, "UPNP_E_FILE_NOT_FOUND"},
    {UPNP_E_FILE_READ_ERROR, "UPNP_E_FILE_READ_ERROR"},
    {UPNP_E_EXT_NOT_XML, "UPNP_E_EXT_NOT_XML"},
    // {UPNP_E_NO_WEB_SERVER, "UPNP_E_NO_WEB_SERVER"},
    // {UPNP_E_OUTOF_BOUNDS, "UPNP_E_OUTOF_BOUNDS"},
    {UPNP_E_NOT_FOUND, "UPNP_E_NOT_FOUND"},
    {UPNP_E_INTERNAL_ERROR, "UPNP_E_INTERNAL_ERROR"},
};

} // namespace

namespace upnplib {

/*!
 * Search for the string name of an error number and return it.
 */
const std::string errStr(const int error) {
    size_t i;
    std::string error_msg = "UPNPLIB_E_UNKNOWN";

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (error == ErrorMessages[i].rc) {
            error_msg = ErrorMessages[i].rcError;
            break;
        }
    }
    return error_msg + "(" + std::to_string(error) + ")";
}

/*!
 * Search for the string names of two error numbers, the wrong and the right
 * one, and return an error message with a hint what is expected.
 */
const std::string errStrEx(const int error, const int success) {
    size_t i;
    std::string error_msg = "UPNPLIB_E_UNKNOWN";
    std::string success_msg = "UPNPLIB_E_UNKNOWN";

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (error == ErrorMessages[i].rc) {
            error_msg = ErrorMessages[i].rcError;
            break;
        }
    }
    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (success == ErrorMessages[i].rc) {
            success_msg = ErrorMessages[i].rcError;
            break;
        }
    }
    return "  # Should be " + success_msg + "(" + std::to_string(success) +
           "), but not " + error_msg + "(" + std::to_string(error) + ").";
}

} // namespace upnplib
