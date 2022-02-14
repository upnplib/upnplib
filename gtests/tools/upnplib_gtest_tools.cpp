// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-02-14

// Tools and helper classes to manage gtests
// =========================================

#include "upnplib_gtest_tools.hpp"
#include "upnp.h" // for UPNP_E_* constants

#include <iostream>
#include <string.h>

namespace upnplib {

// Errormessages taken from https://github.com/pupnp/pupnp
// Author: Marcelo Roberto Jimenez <marcelo.jimenez@gmail.com>
//------------------------------------------------------------
/*!
 * \brief Structure to maintain a error code and string associated with the
 * error code.
 */
struct ErrorString {
    /*! Error code. */
    int rc;
    /*! Error description. */
    const char* rcError;
};

/*!
 * \brief Array of error structures.
 */
struct ErrorString ErrorMessages[] = {
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

const char* UpnpGetErrorMessage(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return ErrorMessages[i].rcError;
        }
    }

    return "Unknown error code";
}

//
// class CCaptureStdOutErr definition
// ----------------------------------
CCaptureStdOutErr::CCaptureStdOutErr(int a_fileno) {
    if (a_fileno != STDOUT_FILENO && a_fileno != STDERR_FILENO) {
        throw ::std::invalid_argument(::std::string(
            (::std::string)__FILE__ + ":" + ::std::to_string(__LINE__) +
            ", constructor " + __func__ +
            ". Only STDOUT_FILENO and STDERR_FILENO supported. "
            "Nothing will be captured."));
    }
    // make a pipe
#ifdef _WIN32
    int rc = ::_pipe(m_out_pipe, m_chunk_size, O_TEXT);
#else
    int rc = ::pipe(m_out_pipe);
#endif
    if (rc != 0) {
        throw ::std::runtime_error(::std::string(
            (::std::string)__FILE__ + ":" + ::std::to_string(__LINE__) +
            ", constructor " + __func__ + ". Creating a pipe failed. " +
            (::std::string)strerror(errno) + '.'));
    }
    m_std_fileno = a_fileno;
    m_saved_stdno =
        ::dup(a_fileno); // save stdout/stderr to restore after capturing
}

//
CCaptureStdOutErr::~CCaptureStdOutErr() {
    ::close(m_out_pipe[0]);
    ::close(m_out_pipe[1]);
}

//
void CCaptureStdOutErr::start() {
    ::dup2(m_out_pipe[1], m_std_fileno); // redirect stdout/stderr to the pipe
}

//
::std::string CCaptureStdOutErr::get() {
    // We always write a nullbyte to the pipe so read always returns
    // and does not wait endless if there is nothing captured.
    const char nullbyte[1] = {'\0'};
    if (::write(m_std_fileno, &nullbyte, 1) == -1)
        return "";

    // read from pipe into chunk and append the chunk to a string
    char chunk[m_chunk_size];
    ::std::string strbuffer{};
    while (::read(m_out_pipe[0], &chunk, m_chunk_size - 1) > 1) {
        strbuffer += chunk;
        if (::write(m_std_fileno, &nullbyte, 1) == -1)
            break;
    }

    // reconnect stdout/stderr
    ::dup2(m_saved_stdno, m_std_fileno);

    return strbuffer;
}

} // namespace upnplib
