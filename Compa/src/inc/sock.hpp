#ifndef COMPA_GENLIB_NET_SOCK_HPP
#define COMPA_GENLIB_NET_SOCK_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-11
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * - Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************/
// Last compare with ./pupnp source file on 2023-09-13, ver 1.14.18
//
// Unused interface header file deleted. It's available until commit
// a01e0186bc0dcb8dc243f6d313ca8527535648fd:compa/inc/compa/sock.hpp
/*!
 * \file
 * \brief \copybrief Sock
 *
 * \defgroup Sock Network Socket Library
 * \brief Manage network sockets and connections.
 * @{
 */

#include <UpnpInet.hpp> /* for SOCKET, netinet/in */
#if defined(UPNP_ENABLE_OPEN_SSL) || defined(DOXYGEN_RUN)
#include <openssl/ssl.h>
#endif

/* The following are not defined under winsock.h */
/*! \todo Cleanup constants: In <sys/socket.h> are defined SHUT_RD, SHUT_WR,
 * SHUT_RDWR with 0, 1, 2, according to man 2 shutdown.
 * \cond */
#ifndef SD_RECEIVE
#define SD_RECEIVE 0x00
#define SD_SEND 0x01
#define SD_BOTH 0x02
#endif
/// \endcond

/// \brief Additional socket information for connections and ssl.
struct SOCKINFO {
    /// \brief Handle/descriptor to a socket.
    SOCKET socket;
    /*! \brief Socket address of the remote node only filled in incoming
     * requests. */
    sockaddr_storage foreign_sockaddr;
#if defined(UPNP_ENABLE_OPEN_SSL) || defined(DOXYGEN_RUN)
    /*! \brief information about an ssl connection only filled in incoming
     * requests. */
    SSL* ssl;
#else
    void* ssl;
#endif
};

/*!
 * \brief Closes the socket if it is different from -1.
 *
 * \return -1 if an error occurred or if the socket is -1.
 */
inline int sock_close(
    /*! Socket descriptor. */
    SOCKET sock) {
    if (sock == INVALID_SOCKET)
        return -1;
    return UpnpCloseSocket(sock);
}

/*!
 * \brief Assign the passed in socket descriptor to socket descriptor in the
 * SOCKINFO structure.
 *
 * \return Integer:
 * \li \c UPNP_E_SUCCESS
 * \li \c UPNP_E_OUTOF_MEMORY
 * \li \c UPNP_E_SOCKET_ERROR
 */
UPNPLIB_API int sock_init(
    /*! [out] Socket Information Object. */
    SOCKINFO* info,
    /*! [in] Socket Descriptor. */
    SOCKET sockfd);

/*!
 * \brief Calls the sock_init function and assigns the passed in IP address
 * and port to the IP address and port in the SOCKINFO structure.
 *
 * \return Integer:
 * \li \c UPNP_E_SUCCESS
 * \li \c UPNP_E_OUTOF_MEMORY
 * \li \c UPNP_E_SOCKET_ERROR
 */
UPNPLIB_API int sock_init_with_ip(
    /*! [out] Socket Information Object. */
    SOCKINFO* info,
    /*! [in] Socket Descriptor. */
    SOCKET sockfd,
    /*! [in] Remote socket address. */
    struct sockaddr* foreign_sockaddr);

/*!
 * \brief Associates an SSL object with the socket and begins
 * the client-side SSL/TLS handshake.
 *
 * \return Integer:
 * \li \c UPNP_E_SUCCESS
 * \li \c UPNP_E_SOCKET_ERROR
 */
#ifdef UPNP_ENABLE_OPEN_SSL
UPNPLIB_API int sock_ssl_connect(
    /*! [out] Socket Information Object. */
    SOCKINFO* info);
#endif

/*!
 * \brief Shutsdown the socket using the ShutdownMethod to indicate whether
 * sends and receives on the socket will be dis-allowed.
 *
 * After shutting down the socket, closesocket is called to release system
 * resources used by the socket calls.
 *
 * \return Integer:
 * \li \c UPNP_E_SOCKET_ERROR on failure.
 * \li \c UPNP_E_SUCCESS on success.
 */
UPNPLIB_API int sock_destroy(
    /*! [in,out] Socket Information Object. */
    SOCKINFO* info,
    /*! [in] How to shutdown the socket. Used by sockets's shutdown(). */
    int ShutdownMethod);

/*!
 * \brief Reads data on socket in sockinfo.
 *
 * \return Integer:
 * \li \c numBytes - On Success, no of bytes received.
 * \li \c UPNP_E_TIMEDOUT - Timeout.
 * \li \c UPNP_E_SOCKET_ERROR - Error on socket calls.
 */
UPNPLIB_API int sock_read(
    /*! [in] Socket Information Object. */
    SOCKINFO* info,
    /*! [out] Buffer to get data to. */
    char* buffer,
    /*! [in] Size of the buffer. */
    size_t bufsize,
    /*! [in,out] timeout value. */
    int* timeoutSecs);

/*!
 * \brief Writes data on the socket in sockinfo.
 *
 * \return Integer:
 * \li \c numBytes - On Success, no of bytes sent.
 * \li \c UPNP_E_TIMEDOUT - Timeout.
 * \li \c UPNP_E_SOCKET_ERROR - Error on socket calls.
 */
UPNPLIB_API int sock_write(
    /*! [in] Socket Information Object. */
    SOCKINFO* info,
    /*! [in] Buffer to send data from. */
    const char* buffer,
    /*! [in] Size of the buffer. */
    size_t bufsize,
    /*! [in,out] timeout value. */
    int* timeoutSecs);

/*!
 * \brief Make socket blocking.
 *
 * \return 0 if successful, -1 otherwise.
 */
UPNPLIB_API int sock_make_blocking(
    /* [in] socket. */
    SOCKET sock);

/*!
 * \brief Make socket non-blocking.
 *
 * \return 0 if successful, -1 otherwise.
 */
UPNPLIB_API int sock_make_no_blocking(
    /* [in] socket. */
    SOCKET sock);

/// @} // Sock Network Socket Library

#endif /* COMPA_GENLIB_NET_SOCK_HPP */
