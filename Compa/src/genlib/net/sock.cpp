/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-09
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
/*!
 * \file
 * \ingroup compa-Addressing
 * \brief Manage network sockets and connections.
 */

#include <sock.hpp>
#include <upnp.hpp>

#include <upnplib/global.hpp>
#include <upnplib/connection.hpp>
#include <upnplib/socket.hpp> // needed for compiling on win32.

#include <umock/sys_socket.hpp>
#include <umock/ssl.hpp>

/// \cond
#include <fcntl.h> /* for F_GETFL, F_SETFL, O_NONBLOCK */
#include <iostream>
#include <cstring>
/// \endcond


namespace {

#ifdef UPNP_ENABLE_OPEN_SSL
/*! \brief Pointer to an SSL Context.
 *
 * Only this one is supported. With the given functions there is no way to use
 * more than this SSL Context. */
SSL_CTX* gSslCtx{nullptr};
#endif

/*! \name Scope restricted to file
 * @{
 */

#if 0
// This is defined on MacOS
#if defined(SO_NOSIGPIPE) || defined(DOXYGEN_RUN)
/// \brief Helper class to unravel the code.
class CNosigpipe {
  public:
    CNosigpipe(SOCKET a_sockfd) : m_sockfd(a_sockfd) {
        // Save old option settings and set SO_NOSIGPIPE
        TRACE2(this, " Construct CNosigpipe()")
        int set{1};
        socklen_t olen{sizeof(old)};
        umock::sys_socket_h.getsockopt(m_sockfd, SOL_SOCKET, SO_NOSIGPIPE, &old,
                                       &olen);
        umock::sys_socket_h.setsockopt(m_sockfd, SOL_SOCKET, SO_NOSIGPIPE, &set,
                                       sizeof(set));
    }
    ~CNosigpipe() {
        // Restore old option settings
        TRACE2(this, " Destruct CNosigpipe()")
        umock::sys_socket_h.setsockopt(m_sockfd, SOL_SOCKET, SO_NOSIGPIPE, &old,
                                       sizeof(old));
    }

  private:
    SOCKET m_sockfd;
    int old{};
};
#endif

/*!
 * \brief Receives or sends data on one unicast link. That means the Unit
 * manages only one socket file descriptor. Also returns the time taken to
 * receive or send data.
 *
 * \return
 *	\li \c numBytes - On Success, number of bytes received or sent or
 *	\li \c UPNP_E_TIMEDOUT - Timeout
 *	\li \c UPNP_E_SOCKET_ERROR - Error on socket calls
 *	\li \c UPNP_E_SOCKET_WRITE - Error on send data
 */
static int sock_read_write(
    /*! [in] Socket Information Object. */
    SOCKINFO* info,
    /*! [out] Buffer to get data to or send data from. */
    char* buffer,
    /*! [in] Size of the buffer. */
    size_t bufsize,
    /*! [in] timeout value: < 0 blocks indefinitely waiting for a file
                                descriptor to become ready. */
    int* timeoutSecs,
    /*! [in] Boolean value specifying read or write option. */
    bool bRead) {
    TRACE("Executing sock_read_write()")
    if (info == nullptr || buffer == nullptr)
        return UPNP_E_SOCKET_ERROR;
    if (bufsize == 0)
        return 0;

    int retCode;
    fd_set readSet;
    fd_set writeSet;
    long numBytes;
    time_t start_time{time(NULL)};
    SOCKET sockfd{info->socket};
    ssize_t num_written;

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_SET(sockfd, bRead ? &readSet : &writeSet);

    // timeoutSecs == nullptr means default timeout to use.
    int timeout_secs =
        (timeoutSecs == nullptr) ? upnplib::g_response_timeout : *timeoutSecs;

    upnplib::CSocketError sockerrObj;
    while (true) {
        // Due to 'man select' timeout should be considered to be undefined
        // after select() returns so we must set it on every loop.
        timeval timeout;
        timeout.tv_sec = timeout_secs;
        timeout.tv_usec = 0;

        // select monitors only one socket file descriptor.
        retCode = umock::sys_socket_h.select(
            static_cast<int>(sockfd + 1), &readSet, &writeSet, NULL,
            (timeout_secs < 0) ? NULL : &timeout);

        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == SOCKET_ERROR) {
            sockerrObj.catch_error();
            if (sockerrObj == EINTRP)
                // Signal catched by select(). It is not for us so we
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* read or write. */
            break;
    }

    if (bRead) {
#ifdef UPNP_ENABLE_OPEN_SSL
        if (info->ssl) {
            // Typecasts are needed to call the SSL function and should not
            // be an issue.
            TRACE("Read data with syscall ::SSL_read().")
            numBytes =
                SSL_read(info->ssl, buffer, static_cast<SIZEP_T>(bufsize));
        } else {
#endif
            TRACE("Read data with syscall ::recv().")
            numBytes = umock::sys_socket_h.recv(
                sockfd, buffer, static_cast<SIZEP_T>(bufsize), 0);
#ifdef UPNP_ENABLE_OPEN_SSL
        }
#endif

    } else { // write network data

        size_t byte_left{bufsize};
        long bytes_sent{};
#ifdef SO_NOSIGPIPE // This is defined on MacOS
        CNosigpipe nosigpipe(sockfd); // Save, set and restore option settings.
#endif
        while (byte_left != static_cast<size_t>(0)) {
#ifdef UPNP_ENABLE_OPEN_SSL
            if (info->ssl) {
                TRACE("Write data with syscall ::SSL_write().")
                num_written = SSL_write(info->ssl, buffer + bytes_sent,
                                        static_cast<int>(byte_left));
            } else {
#endif
                TRACE("Write data with syscall ::send().")
                num_written =
                    umock::sys_socket_h.send(sockfd, buffer + bytes_sent,
                                             static_cast<SIZEP_T>(byte_left),
                                             MSG_DONTROUTE | MSG_NOSIGNAL);
#ifdef UPNP_ENABLE_OPEN_SSL
            }
#endif
            if (num_written == -1) {
                return UPNP_E_SOCKET_WRITE;
            }
            byte_left -= static_cast<size_t>(num_written);
            bytes_sent += static_cast<long>(num_written);
        }
        numBytes = bytes_sent;
    }
    if (numBytes < 0)
        return UPNP_E_SOCKET_ERROR;

    /* subtract time used for reading/writing. */
    if (timeoutSecs != nullptr && timeout_secs != 0)
        *timeoutSecs -= static_cast<int>(time(NULL) - start_time);

    return numBytes;
}
#endif // #if 0

/*!
 * \brief Read from a not SSL protected socket.
 *
 * \returns
 *  On success: Number of bytes read. 0 bytes read is no error.\n
 *  On error:
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_TIMEDOUT
 */
int sock_read_unprotected(
    /*! [in] Socket Information Object. */
    const SOCKINFO* a_info,
    /*! [out] Buffer to get data to. */
    char* a_readbuf,
    /*! [in] Buffer to send data from. */
    [[maybe_unused]] const char* a_writebuf,
    /*! [in] Size of the used buffer (read or write). */
    const size_t a_bufsize,
    /*! [in] timeout value: < 0 blocks indefinitely waiting for a file
                                descriptor to become ready. */
    int* a_timeoutSecs) {
    time_t start_time{time(NULL)};
    TRACE("Executing sock_read_unprotected()")

    // Also restrict a_bufsize to integer for save later use despite type cast.
    if (a_info == nullptr || a_readbuf == nullptr || a_bufsize > INT_MAX)
        return UPNP_E_SOCKET_ERROR;
    if (a_bufsize == 0)
        return 0;

    SOCKET sockfd{a_info->socket};

    ::fd_set readSet;
    ::fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_SET(sockfd, &readSet);

    // a_timeoutSecs == nullptr means default timeout to use.
    int timeout_secs = (a_timeoutSecs == nullptr) ? upnplib::g_response_timeout
                                                  : *a_timeoutSecs;

    upnplib::CSocketError sockerrObj;
    while (true) {
        // Due to 'man select' timeout should be considered to be undefined
        // after select() returns so we must set it on every loop.
        ::timeval timeout;
        timeout.tv_sec = timeout_secs;
        timeout.tv_usec = 0;

        // select() monitors only one socket file descriptor.
        int retCode = umock::sys_socket_h.select(
            static_cast<int>(sockfd + 1), &readSet, &writeSet, nullptr,
            (timeout_secs < 0) ? nullptr : &timeout);

        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == SOCKET_ERROR) {
            sockerrObj.catch_error();
            if (sockerrObj == EINTRP)
                // Signal catched by select(). It is not for us so we
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* read */
            break;
    }

    TRACE("Read data with syscall ::recv().")
    SSIZEP_T numBytes = umock::sys_socket_h.recv(
        sockfd, a_readbuf, static_cast<SIZEP_T>(a_bufsize), 0);

    // Also protect type cast
    if (numBytes < 0 || numBytes > INT_MAX)
        return UPNP_E_SOCKET_ERROR;

    /* subtract time used for reading/writing. */
    if (a_timeoutSecs != nullptr && timeout_secs != 0)
        *a_timeoutSecs -= static_cast<int>(time(NULL) - start_time);

    return static_cast<int>(numBytes);
}


/*!
 * \brief Write to a not SSL protected socket.
 *
 * \returns
 *  On success: Number of bytes written. 0 bytes written is no error.\n
 *  On error:
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 */
int sock_write_unprotected(
    /*! [in] Socket Information Object. */
    const SOCKINFO* a_info,
    /*! [out] Buffer to get data to. */
    [[maybe_unused]] char* a_readbuf,
    /*! [in] Buffer to send data from. */
    const char* a_writebuf,
    /*! [in] Size of the buffer. */
    const size_t a_bufsize,
    /*! [in] timeout value: < 0 blocks indefinitely waiting for a file
                                descriptor to become ready. */
    int* a_timeoutSecs) {
    time_t start_time{time(NULL)};
    TRACE("Executing sock_write_unprotected()")

    // Also restrict a_bufsize to integer for save later use despite type cast.
    if (a_info == nullptr || a_writebuf == nullptr || a_bufsize > INT_MAX)
        return UPNP_E_SOCKET_ERROR;
    if (a_bufsize == 0)
        return 0;

    SOCKET sockfd{a_info->socket};

    ::fd_set readSet;
    ::fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_SET(sockfd, &writeSet);

    // a_timeoutSecs == nullptr means default timeout to use.
    int timeout_secs = (a_timeoutSecs == nullptr) ? upnplib::g_response_timeout
                                                  : *a_timeoutSecs;

    upnplib::CSocketError sockerrObj;
    while (true) {
        // Due to 'man select' timeout should be considered to be undefined
        // after select() returns so we must set it on every loop.
        ::timeval timeout;
        timeout.tv_sec = timeout_secs;
        timeout.tv_usec = 0;

        // select monitors only one socket file descriptor.
        int retCode = umock::sys_socket_h.select(
            static_cast<int>(sockfd + 1), &readSet, &writeSet, nullptr,
            (timeout_secs < 0) ? nullptr : &timeout);

        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == SOCKET_ERROR) {
            sockerrObj.catch_error();
            if (sockerrObj == EINTRP)
                // Signal catched by select(). It is not for us so we
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* write. */
            break;
    }

    // a_bufsize is restricted from 0 to INT_MAX.
    ssize_t byte_left{static_cast<ssize_t>(a_bufsize)};
    ssize_t bytes_sent{};

    UPNPLIB_SCOPED_NO_SIGPIPE
    while (byte_left != 0) {
        TRACE("Write data with syscall ::send().")
        ssize_t num_written = umock::sys_socket_h.send(
            sockfd, a_writebuf + bytes_sent, static_cast<SIZEP_T>(byte_left),
            MSG_DONTROUTE);
        if (num_written == -1 || num_written > INT_MAX) {
            return UPNP_E_SOCKET_WRITE;
        }
        byte_left -= num_written;
        bytes_sent += num_written;
    }

    // Also protect type cast
    if (bytes_sent < 0 || bytes_sent > INT_MAX)
        return UPNP_E_SOCKET_ERROR;

    /* subtract time used for writing. */
    if (a_timeoutSecs != nullptr && timeout_secs != 0)
        *a_timeoutSecs -= static_cast<int>(time(NULL) - start_time);

    return static_cast<int>(bytes_sent);
}


#ifdef UPNP_ENABLE_OPEN_SSL
/*!
 * \brief Read from an SSL protected socket.
 *
 * This is only available with OpenSSL enabled on compiling the library.
 *
 * \returns
 *  On success: Number of bytes read. 0 bytes read is no error.\n
 *  On error:
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_TIMEDOUT
 */
int sock_read_ssl(
    /*! [in] Socket Information Object. */
    const SOCKINFO* a_info,
    /*! [out] Buffer to get data to. */
    char* a_readbuf,
    /*! [in] Buffer to send data from. */
    [[maybe_unused]] const char* a_writebuf,
    /*! [in] Size of the used buffer (read or write). */
    const size_t a_bufsize,
    /*! [in] timeout value: < 0 blocks indefinitely waiting for a file
                                descriptor to become ready. */
    int* a_timeoutSecs) {
    time_t start_time{time(NULL)};
    TRACE("Executing sock_read_ssl()")

    // Also restrict a_bufsize to integer for save later use despite type cast.
    if (a_info == nullptr || a_readbuf == nullptr || a_bufsize > INT_MAX)
        return UPNP_E_SOCKET_ERROR;
    if (a_bufsize == 0)
        return 0;

    SOCKET sockfd{a_info->socket};

    ::fd_set readSet;
    ::fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_SET(sockfd, &readSet);

    // a_timeoutSecs == nullptr means default timeout to use.
    int timeout_secs = (a_timeoutSecs == nullptr) ? upnplib::g_response_timeout
                                                  : *a_timeoutSecs;

    upnplib::CSocketError sockerrObj;
    while (true) {
        // Due to 'man select' timeout should be considered to be undefined
        // after select() returns so we must set it on every loop.
        ::timeval timeout;
        timeout.tv_sec = timeout_secs;
        timeout.tv_usec = 0;

        // select() monitors only one socket file descriptor.
        int retCode = umock::sys_socket_h.select(
            static_cast<int>(sockfd + 1), &readSet, &writeSet, nullptr,
            (timeout_secs < 0) ? nullptr : &timeout);

        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == SOCKET_ERROR) {
            sockerrObj.catch_error();
            if (sockerrObj == EINTRP)
                // Signal catched by select(). It is not for us so we
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* read */
            break;
    }

    TRACE("Read data with syscall ::SSL_read().")
    // Type cast a_bufsize is protected above to only contain 0 to INT_MAX.
    int numBytes = umock::ssl_h.SSL_read(a_info->ssl, a_readbuf,
                                         static_cast<int>(a_bufsize));

    if (numBytes < 0)
        return UPNP_E_SOCKET_ERROR;

    /* subtract time used for reading/writing. */
    if (a_timeoutSecs != nullptr && timeout_secs != 0)
        *a_timeoutSecs -= static_cast<int>(time(NULL) - start_time);

    return numBytes;
}


/*!
 * \brief Write to an SSL protected socket.
 *
 * This is only available with OpenSSL enabled on compiling the library.
 *
 * \returns
 *  On success: Number of bytes written. 0 bytes written is no error.\n
 *  On error:
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 */
int sock_write_ssl(
    /*! [in] Socket Information Object. */
    const SOCKINFO* a_info,
    /*! [out] Buffer to get data to. */
    [[maybe_unused]] char* a_readbuf,
    /*! [in] Buffer to send data from. */
    const char* a_writebuf,
    /*! [in] Size of the buffer. */
    const size_t a_bufsize,
    /*! [in] timeout value: < 0 blocks indefinitely waiting for a file
                                descriptor to become ready. */
    int* a_timeoutSecs) {
    time_t start_time{time(NULL)};
    TRACE("Executing sock_write_ssl()")

    // Also restrict a_bufsize to integer for save later use despite type cast.
    if (a_info == nullptr || a_writebuf == nullptr || a_bufsize > INT_MAX)
        return UPNP_E_SOCKET_ERROR;
    if (a_bufsize == 0)
        return 0;

    SOCKET sockfd{a_info->socket};

    ::fd_set readSet;
    ::fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_SET(sockfd, &writeSet);

    // a_timeoutSecs == nullptr means default timeout to use.
    int timeout_secs = (a_timeoutSecs == nullptr) ? upnplib::g_response_timeout
                                                  : *a_timeoutSecs;

    upnplib::CSocketError sockerrObj;
    while (true) {
        // Due to 'man select' timeout should be considered to be undefined
        // after select() returns so we must set it on every loop.
        ::timeval timeout;
        timeout.tv_sec = timeout_secs;
        timeout.tv_usec = 0;

        // select monitors only one socket file descriptor.
        int retCode = umock::sys_socket_h.select(
            static_cast<int>(sockfd + 1), &readSet, &writeSet, nullptr,
            (timeout_secs < 0) ? nullptr : &timeout);

        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == SOCKET_ERROR) {
            sockerrObj.catch_error();
            if (sockerrObj == EINTRP)
                // Signal catched by select(). It is not for us so we
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* write. */
            break;
    }

    int byte_left{static_cast<int>(a_bufsize)};
    int bytes_sent{};

    UPNPLIB_SCOPED_NO_SIGPIPE
    while (byte_left != 0) {
        TRACE("Write data with syscall ::SSL_write().")
        int num_written = umock::ssl_h.SSL_write(
            a_info->ssl, a_writebuf + bytes_sent, byte_left);
        if (num_written == -1) {
            return UPNP_E_SOCKET_WRITE;
        }
        byte_left -= num_written;
        bytes_sent += num_written;
    }

    if (bytes_sent < 0)
        return UPNP_E_SOCKET_ERROR;

    /* subtract time used for writing. */
    if (a_timeoutSecs != nullptr && timeout_secs != 0)
        *a_timeoutSecs -= static_cast<int>(time(NULL) - start_time);

    return bytes_sent;
}
#endif

/// @} // Functions (scope restricted to file)
} // anonymous namespace


int sock_init(SOCKINFO* info, SOCKET sockfd) {
    TRACE("Executing sock_init()")
    if (info == nullptr)
        return UPNP_E_INVALID_PARAM;

    memset(info, 0, sizeof(SOCKINFO));
    info->socket = sockfd;

    return UPNP_E_SUCCESS;
}

int sock_init_with_ip(SOCKINFO* info, SOCKET sockfd,
                      sockaddr* foreign_sockaddr) {
    TRACE("Executing sock_init_with_ip()")

    if (foreign_sockaddr == nullptr)
        return UPNP_E_INVALID_PARAM;

    int ret = sock_init(info, sockfd);
    if (ret != UPNP_E_SUCCESS) {
        return ret;
    }
    memcpy(&info->foreign_sockaddr, foreign_sockaddr,
           sizeof(info->foreign_sockaddr));

    return UPNP_E_SUCCESS;
}

#ifdef UPNP_ENABLE_OPEN_SSL
int sock_ssl_connect(SOCKINFO* info) {
    TRACE("Executing sock_ssl_connect()");
    info->ssl = SSL_new(gSslCtx);
    if (!info->ssl) {
        return UPNP_E_SOCKET_ERROR;
    }
    // Due to man page there is no problem with type cast (int)
    int status = SSL_set_fd(info->ssl, static_cast<int>(info->socket));
    if (status == 0)
        return UPNP_E_SOCKET_ERROR;

    UPNPLIB_SCOPED_NO_SIGPIPE;
    status = SSL_connect(info->ssl);
    if (status != 1)
        return UPNP_E_SOCKET_ERROR;

    return UPNP_E_SUCCESS;
}
#endif

int sock_destroy(SOCKINFO* info, int ShutdownMethod) {
    TRACE("Executing sock_destroy()")
    int ret{UPNP_E_SUCCESS};

    if (info->socket != INVALID_SOCKET) {
#ifdef UPNP_ENABLE_OPEN_SSL
        if (info->ssl) {
            SSL_shutdown(info->ssl);
            SSL_free(info->ssl);
            info->ssl = NULL;
        }
#endif
        upnplib::CSocketError sockerrObj;
        if (umock::sys_socket_h.shutdown(info->socket, ShutdownMethod) ==
            SOCKET_ERROR) {
            sockerrObj.catch_error();
            std::string msg = "MSG1010: syscall ::shutdown() returned \"" +
                              sockerrObj.get_error_str() + "\".\n";
            if (sockerrObj == ENOTCONNP) {
                // shutdown a not connected connection is not an error.
                UPNPLIB_LOGINFO << msg;
            } else {
                UPNPLIB_LOGERR << msg;
                ret = UPNP_E_SOCKET_ERROR;
            }
        }
        if (sock_close(info->socket) != 0) {
            ret = UPNP_E_SOCKET_ERROR;
        }
        info->socket = INVALID_SOCKET;
    }

    return ret;
}


int sock_read(SOCKINFO* info, char* buffer, size_t bufsize, int* timeoutSecs) {
    TRACE("Executing sock_read()")
    if (info == nullptr)
        return UPNP_E_SOCKET_ERROR;
#ifdef UPNP_ENABLE_OPEN_SSL
    if (info->ssl)
        return sock_read_ssl(info, buffer /*read_buffer*/,
                             nullptr /*write_buffer*/, bufsize, timeoutSecs);
    else
#endif
        return sock_read_unprotected(info, buffer /*read_buffer*/,
                                     nullptr /*write_buffer*/, bufsize,
                                     timeoutSecs);
}

int sock_write(SOCKINFO* info, const char* buffer, size_t bufsize,
               int* timeoutSecs) {
    TRACE("Executing sock_write()")
    if (info == nullptr)
        return UPNP_E_SOCKET_ERROR;
#ifdef UPNP_ENABLE_OPEN_SSL
    if (info->ssl)
        return sock_write_ssl(info, nullptr /*read_buffer*/,
                              buffer /*write_buffer*/, bufsize, timeoutSecs);
    else
#endif
        return sock_write_unprotected(info, nullptr /*read_buffer*/,
                                      buffer /*write_buffer*/, bufsize,
                                      timeoutSecs);
}

int sock_make_blocking(SOCKET sock) {
// returns 0 if successful, else SOCKET_ERROR.
#ifdef _WIN32
    u_long val = 0;
    // returns 0 if successful, else SOCKET_ERROR.
    return ioctlsocket(sock, FIONBIO, &val);
#else
    int val = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, val & ~O_NONBLOCK) == -1) {
        return SOCKET_ERROR;
    }
    return 0;
#endif
}

int sock_make_no_blocking(SOCKET sock) {
// returns 0 if successful, else SOCKET_ERROR.
#ifdef _WIN32
    u_long val = 1;
    // returns 0 if successful, else SOCKET_ERROR.
    return ioctlsocket(sock, FIONBIO, &val);
#else  /* _WIN32 */
    int val = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, val | O_NONBLOCK) == -1) {
        return SOCKET_ERROR;
    }
    return 0;
#endif /* _WIN32 */
}

#ifdef UPNP_ENABLE_OPEN_SSL
int UpnpInitSslContext([[maybe_unused]] int initOpenSslLib,
                       const SSL_METHOD* sslMethod) {
    if (gSslCtx)
        return UPNP_E_INIT;
#if 0 // next three OpenSSL functions are deprecated due to its man pages and
      // should not be used since OpenSSL 1.1.0. Initialization is done
      // automatically now. --Ingo
    if (initOpenSslLib) {
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }
#endif
    gSslCtx = SSL_CTX_new(sslMethod);
    if (!gSslCtx) {
        return UPNP_E_INIT_FAILED;
    }
    return UPNP_E_SUCCESS;
}

void freeSslCtx() {
    if (gSslCtx) {
        SSL_CTX_free(gSslCtx);
        gSslCtx = nullptr;
    }
}
#endif
