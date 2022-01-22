// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-13

#include "upnplib/httpreadwrite.hpp"

namespace upnplib {

#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS

/* in seconds */
#define DEFAULT_TCP_CONNECT_TIMEOUT 5

extern size_t g_maxContentLength;

/*!
 * \brief Checks socket connection and wait if it is not connected.
 * It should be called just after connect.
 *
 * \return 0 if successful, else -1.
 */
// This should be the same as used in
// pupnp/src/genlib/net/http/httpreadwrite.cpp
// but only reorganized for better readability.

static int Check_Connect_And_Wait_Connection(
    /*! [in] socket. */
    SOCKET sock,
    /*! [in] result of connect. */
    int connect_res) //
{
    struct timeval tmvTimeout = {DEFAULT_TCP_CONNECT_TIMEOUT, 0};
    int result;

#ifdef _WIN32
    struct fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(sock, &fdSet);

    if (connect_res < 0) {
        if (WSAEWOULDBLOCK == upnplib::winsock2_h->WSAGetLastError()) {
            result = upnplib::sys_select_h->select(sock + 1, NULL, &fdSet, NULL,
                                                   &tmvTimeout);
            if (result < 0) {
                return -1;
            } else if (result == 0) {
                /* timeout */
                return -1;
            }
        }
        // BUG! It should not return 0 if we have unexpected 'connect()' errno
    }

    return 0;
}
#else  // _WIN32

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(sock, &fdSet);

    if (connect_res < 0) {
        if (EINPROGRESS == errno) {
            result = upnplib::sys_select_h->select(sock + 1, NULL, &fdSet, NULL,
                                                   &tmvTimeout);
            if (result < 0) {
                return -1;
            } else if (result == 0) {
                /* timeout */
                return -1;
            } else {
                int valopt = 0;
                socklen_t len = sizeof(valopt);
                if (upnplib::sys_socket_h->getsockopt(
                        sock, SOL_SOCKET, SO_ERROR, (void*)&valopt, &len) < 0) {
                    /* failed to read delayed error */
                    return -1;
                } else if (valopt) {
                    /* delayed error = valopt */
                    return -1;
                }
            }
        }
        // BUG! It should not return 0 if we have unexpected 'connect()' errno
    }

    return 0;
}
#endif // _WIN32
#endif /* UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS */

} // namespace upnplib
