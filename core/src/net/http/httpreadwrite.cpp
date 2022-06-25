// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-02-24

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

[[maybe_unused]] static int Check_Connect_And_Wait_Connection(
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
                    // TODO: Return more detailed error codes, e.g.
                    // valopt == 111: ECONNREFUSED "Connection refused"
                    // if there is a remote host but no server service
                    // listening.
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

//
CUri::CUri(std::string a_url_str) : url_str(a_url_str), hostport{} {
    // Exception: no
    const auto start = this->url_str.find("://");
    if (start == std::string::npos)
        throw std::invalid_argument(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ + ". '://' not found in url."));

    // Exception: no
    const auto end = this->url_str.find_first_of("/", start + 3);
    if (end == std::string::npos)
        throw std::invalid_argument(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", constructor " + __func__ +
                        ". hostport delimiter '/' not found in url."));

    const auto hostport_size = end - start - 3;
    if (hostport_size == 0)
        throw std::invalid_argument(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ + ". 'No hostport found in url."));

    // Exception: std::out_of_range if pos > size()
    this->hostport = this->url_str.substr(start + 3, hostport_size);
}

} // namespace upnplib
