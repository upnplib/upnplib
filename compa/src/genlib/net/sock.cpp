/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2023-12-02
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
 * \addtogroup Sock
 *
 * @{
 *
 * \file
 *
 * \brief Implements the sockets functionality.
 */

#include <config.hpp>

#include <sock.hpp>
#include <unixutil.hpp> /* for socklen_t, EAFNOSUPPORT */
#include <upnp.hpp>

#include <upnpdebug.hpp>
#include <upnputil.hpp>

#include <assert.h>
#include <cerrno>
#include <fcntl.h> /* for F_GETFL, F_SETFL, O_NONBLOCK */
#include <cstring>
#include <ctime>
#include <iostream>

#include <upnplib/general.hpp>
#include <upnplib/socket.hpp>

#ifndef _WIN32
#include <csignal>
#endif

#include <umock/sys_socket.hpp>

#ifdef UPNP_ENABLE_OPEN_SSL
#include <openssl/ssl.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifdef UPNP_ENABLE_OPEN_SSL
/* OpenSSL context defined in upnpapi.c */
UPNPLIB_EXTERN SSL_CTX* gSslCtx;
#endif

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
    int status = 0;
    info->ssl = SSL_new(gSslCtx);
    if (!info->ssl) {
        return UPNP_E_SOCKET_ERROR;
    }
    // Typecast from SOCKET to int is no problem due to SSL man page.
    status = SSL_set_fd(info->ssl, (int)info->socket);
    if (status == 1) {
        status = SSL_connect(info->ssl);
    }
    if (status == 1) {
        return UPNP_E_SUCCESS;
    }
    return UPNP_E_SOCKET_ERROR;
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


#ifdef SO_NOSIGPIPE // This is defined on MacOS
// Helper class to unravel the code.
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
    timeval timeout;
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
    timeout.tv_sec = timeout_secs;
    timeout.tv_usec = 0;

    upnplib::CSocketError sockerrObj;
    while (true) {
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
#ifdef SO_NOSIGPIPE                   // This is defined on MacOS
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
    /* return time used for reading/writing. */
    if (timeoutSecs != nullptr && timeout_secs != 0)
        *timeoutSecs = static_cast<int>(time(NULL) - start_time);

    return numBytes;
}

int sock_read(SOCKINFO* info, char* buffer, size_t bufsize, int* timeoutSecs) {
    TRACE("Executing sock_read()")
    return sock_read_write(info, buffer, bufsize, timeoutSecs, true);
}

int sock_write(SOCKINFO* info, const char* buffer, size_t bufsize,
               int* timeoutSecs) {
    TRACE("Executing sock_write()")
    /* Consciently removing constness. */
    return sock_read_write(info, (char*)buffer, bufsize, timeoutSecs, false);
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


namespace compa {

class CSigpipe {
    // Taking the idea and example for this SIGPIPE handler from
    // https://stackoverflow.com/a/2347848/5014688 and adapt it.
    // Thanks to kroki.
    // This is a general solution, for when you are not in full control of the
    // program’s signal handling and want to write data to an actual pipe or
    // use write(2) on a socket. To control SIGPIPE on send(2) I use socket
    // option MSG_NOSIGNAL on Linux and SO_NOSIGPIPE on MacOS. But with SSL
    // function calls we have to use this class. --Ingo

#if !defined __APPLE__ && !defined _WIN32
  private:
    sigset_t sigpipe_mask;
    bool sigpipe_pending;
    bool sigpipe_unblock;

  public:
    CSigpipe() {
        // The only error that can be returned in errno is 'EINVAL signum is not
        // a valid signal.' SIGPIPE is always a valid signum, so no error
        // handling is required.
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);
    }
#endif

  public:
    void suppress([[maybe_unused]] SOCKET sockfd) {
#ifdef _WIN32
        // Nothing to do for Microsoft Windows. It does not invoke a signal.
#elif __APPLE__
        // On MacOS I set the SO_NOSIGPIPE option for send() with the socket.
#else
        /*
          We want to ignore possible SIGPIPE that we can generate on write.
          SIGPIPE is delivered *synchronously* and *only* to the thread doing
          the write. So if it is reported as already pending (which means the
          thread blocks it), then we do nothing: if we generate SIGPIPE, it will
          be merged with the pending one (there's no queuing), and that suits us
          well. If it is not pending, we block it in this thread (and we avoid
          changing signal action, because it is per-process).
        */
        sigset_t pending;
        sigemptyset(&pending);
        sigpending(&pending);

        sigpipe_pending = sigismember(&pending, SIGPIPE);
        if (!sigpipe_pending) {
            sigset_t blocked;
            sigemptyset(&blocked);
            pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &blocked);

            /* Maybe is was blocked already?  */
            sigpipe_unblock = !sigismember(&blocked, SIGPIPE);
        }
#endif
    }

    void restore() {
#if !defined __APPLE__ && !defined _WIN32
        /*
          If SIGPIPE was pending already we do nothing. Otherwise, if it become
          pending (i.e., we generated it), then we sigwait() it (thus clearing
          pending status). Then we unblock SIGPIPE, but only if it were us who
          blocked it.
        */
        if (!sigpipe_pending) {
            sigset_t pending;
            sigemptyset(&pending);
            sigpending(&pending);
            if (sigismember(&pending, SIGPIPE)) {
                /*
                  Protect ourselves from a situation when SIGPIPE was sent by
                  the user to the whole process, and was delivered to other
                  thread before we had a chance to wait for it.
                */
                static const struct timespec nowait = {0, 0};
                int sig{-1};
                do
                    sig = sigtimedwait(&sigpipe_mask, NULL, &nowait);
                while (sig == -1 && errno == EINTR);
            }

            if (sigpipe_unblock)
                pthread_sigmask(SIG_UNBLOCK, &sigpipe_mask, NULL);
        }
#endif
    }
};


#ifdef UPNP_ENABLE_OPEN_SSL
int sock_ssl_connect(SOCKINFO* info) {
    int status{};
    info->ssl = SSL_new(gSslCtx);
    if (!info->ssl) {
        return UPNP_E_SOCKET_ERROR;
    }
    // Due to man page there is no problem with type cast (int)
    status = SSL_set_fd(info->ssl, (int)info->socket);
    if (status == 1) {
        CSigpipe sigpipe;
        sigpipe.suppress(info->socket);
        status = SSL_connect(info->ssl);
        sigpipe.restore();
    }
    if (status == 1) {
        return UPNP_E_SUCCESS;
    }
    return UPNP_E_SOCKET_ERROR;
}
#endif

} // namespace compa

/* @} Sock */
