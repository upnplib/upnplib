// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-16

// Tools and helper classes to manage gtests
// =========================================

#include "custom_gtest_tools_all.hpp"
#include "port.hpp"
#include "port_unistd.hpp"
#include "upnp.h" // for UPNP_E_* constants

#include <iostream>
#include <string.h>

namespace upnp {

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
CCaptureStdOutErr::CCaptureStdOutErr(int t_fileno) {
    if (t_fileno != STDOUT_FILENO && t_fileno != STDERR_FILENO) {
        m_error = true;
        std::cerr << "\n[ ERROR    ] " << __FILE__ << ", Line " << __LINE__
                  << ", constructor: Only STDOUT_FILENO and STDERR_FILENO "
                     "supported. Nothing will be captured."
                  << std::endl;
        return;
    }
    // make a pipe
#ifdef _WIN32
    int rc = ::_pipe(m_out_pipe, UPNP_PIPE_BUFFER_SIZE, O_TEXT);
#else
    int rc = ::pipe(m_out_pipe);
#endif
    if (rc != 0) {
        m_error = true;
        std::cerr << "\n[ ERROR    ] " << __FILE__ << ", Line " << __LINE__
                  << ", constructor: Creating a pipe failed. "
                  << strerror(errno) << std::endl;
        return;
    }
    m_std_fileno = t_fileno;
    m_saved_stdno = ::dup(t_fileno); // save stderr to restore after capturing
}

bool CCaptureStdOutErr::start() {
    if (m_error)
        return false;

    ::dup2(m_out_pipe[1], m_std_fileno); // redirect stderr to the pipe
    return true;
}

bool CCaptureStdOutErr::get(std::string& t_captured) {
    if (m_error)
        return false;

    char capture_buffer[UPNP_PIPE_BUFFER_SIZE]{};

    // We always write a nullbyte to the pipe so read always returns
    // and does not wait endless if there is nothing captured.
    const char nullbyte[1] = {'\0'};
    ::write(m_std_fileno, &nullbyte, 1);

    // read from pipe into capture_buffer
    ssize_t count =
        ::read(m_out_pipe[0], &capture_buffer, sizeof(capture_buffer) - 1);

    // reconnect stderr
    ::dup2(m_saved_stdno, m_std_fileno);

    if (count == sizeof(capture_buffer) - 1) {
        // Not all characters read
        std::cerr << "\n[ ERROR    ] " << __FILE__ << ", Line " << __LINE__
                  << ", method " << __func__
                  << ": capture_buffer to small, captured characters may be "
                     "lost. You must increase it."
                  << std::endl;
        return false;
    }

    // Return the character buffer as string object.
    std::string s = capture_buffer;
    t_captured = s;
    return true;
}

} // namespace upnp
