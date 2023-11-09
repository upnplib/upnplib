/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2023-11-17
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
#include <compa/sock.hpp>

#include <unixutil.hpp> /* for socklen_t, EAFNOSUPPORT */
#include <upnp.hpp>

#include <upnpdebug.hpp>
#include <upnputil.hpp>

#include <assert.h>
#include <cerrno>
#include <fcntl.h> /* for F_GETFL, F_SETFL, O_NONBLOCK */
#include <cstring>
#include <ctime>

#ifdef UPNPLIB_WITH_TRACE
#include <iostream>
#endif
#include <upnplib/general.hpp>

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
    assert(info);

    memset(info, 0, sizeof(SOCKINFO));
    info->socket = sockfd;

    return UPNP_E_SUCCESS;
}

int sock_init_with_ip(SOCKINFO* info, SOCKET sockfd,
                      struct sockaddr* foreign_sockaddr) {
    int ret;

    ret = sock_init(info, sockfd);
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
    int ret = UPNP_E_SUCCESS;
    char errorBuffer[ERROR_BUFFER_LEN];

    if (info->socket != INVALID_SOCKET) {
#ifdef UPNP_ENABLE_OPEN_SSL
        if (info->ssl) {
            SSL_shutdown(info->ssl);
            SSL_free(info->ssl);
            info->ssl = NULL;
        }
#endif
        if (umock::sys_socket_h.shutdown(info->socket, ShutdownMethod) == -1) {
            strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
            UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                       "Error in shutdown: %s\n", errorBuffer);
        }
        // BUG! closesocket on _WIN32 does not return -1 on error, but positive
        // numbers. This must check != 0. --Ingo
        if (sock_close(info->socket) == -1) {
            ret = UPNP_E_SOCKET_ERROR;
        }
        info->socket = INVALID_SOCKET;
    }

    return ret;
}

/*!
 * \brief Receives or sends data. Also returns the time taken to receive or
 * send data.
 *
 * \return
 *	\li \c numBytes - On Success, no of bytes received or sent or
 *	\li \c UPNP_E_TIMEDOUT - Timeout
 *	\li \c UPNP_E_SOCKET_ERROR - Error on socket calls
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
    int retCode;
    fd_set readSet;
    fd_set writeSet;
    struct timeval timeout;
    long numBytes;
    time_t start_time = time(NULL);
    SOCKET sockfd = info->socket;
    long bytes_sent = 0;
    size_t byte_left = 0;
    ssize_t num_written;

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    if (bRead)
        FD_SET(sockfd, &readSet);
    else
        FD_SET(sockfd, &writeSet);
    timeout.tv_sec = *timeoutSecs;
    timeout.tv_usec = 0;
    while (1) {
        // BUG! To get correct error messages from the system call for checking
        // signals EINTR (see below) the errno must be resetted. I have seen an
        // endless loop here with old contents of errno.  errno = 0; --Ingo
        if (*timeoutSecs < 0)
            retCode = umock::sys_socket_h.select((int)sockfd + 1, &readSet,
                                                 &writeSet, NULL, NULL);
        else
            retCode = umock::sys_socket_h.select((int)sockfd + 1, &readSet,
                                                 &writeSet, NULL, &timeout);
        if (retCode == 0)
            return UPNP_E_TIMEDOUT;
        if (retCode == -1) {
            if (errno == EINTR)
                continue;
            return UPNP_E_SOCKET_ERROR;
        } else
            /* read or write. */
            break;
    }
#ifdef SO_NOSIGPIPE
    {
        int old;
        int set = 1;
        socklen_t olen = sizeof(old);
        umock::sys_socket_h.getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &old,
                                       &olen);
        umock::sys_socket_h.setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &set,
                                       sizeof(set));
#endif
        if (bRead) {
#ifdef UPNP_ENABLE_OPEN_SSL
            if (info->ssl) {
                // Typecasts are needed to call the SSL function and should not
                // be an issue.
                numBytes = (long)SSL_read(info->ssl, buffer, (SIZEP_T)bufsize);
            } else {
#endif
                /* read data. */
                numBytes = (long)umock::sys_socket_h.recv(
                    sockfd, buffer, (SIZEP_T)bufsize, MSG_NOSIGNAL);
#ifdef UPNP_ENABLE_OPEN_SSL
            }
#endif
        } else {
            byte_left = bufsize;
            bytes_sent = 0;
            while (byte_left != (size_t)0) {
#ifdef UPNP_ENABLE_OPEN_SSL
                if (info->ssl) {
                    num_written = SSL_write(info->ssl, buffer + bytes_sent,
                                            (int)byte_left);
                } else {
#endif
                    /* write data. */
                    num_written = umock::sys_socket_h.send(
                        sockfd, buffer + bytes_sent, (SIZEP_T)byte_left,
                        MSG_DONTROUTE | MSG_NOSIGNAL);
#ifdef UPNP_ENABLE_OPEN_SSL
                }
#endif
                if (num_written == -1) {
#ifdef SO_NOSIGPIPE
                    umock::sys_socket_h.setsockopt(sockfd, SOL_SOCKET,
                                                   SO_NOSIGPIPE, &old, olen);
#endif
                    // BUG! Should return UPNP_E_SOCKET_ERROR --Ingo
                    return (int)num_written;
                }
                byte_left -= (size_t)num_written;
                bytes_sent += (long)num_written;
            }
            numBytes = bytes_sent;
        }
#ifdef SO_NOSIGPIPE
        umock::sys_socket_h.setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &old,
                                       olen);
    }
#endif
    if (numBytes < 0)
        return UPNP_E_SOCKET_ERROR;
    /* subtract time used for reading/writing. */
    if (*timeoutSecs != 0)
        *timeoutSecs -= (int)(time(NULL) - start_time);

    return (int)numBytes;
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
    // https://stackoverflow.com/a/2347848/5014688 and adapt it. Thanks to
    // kroki. --Ingo

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
// There is nothing to do with Microsoft Windows. It does not invoke a signal.
#elif __APPLE__
        // On MacOS we set the SO_NOSIGPIPE option on the socket.
        // (seems it is not needed? We will see with extended tests.) --Ingo
        // int set = 1;
        // setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(int));
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
int Csock::sock_ssl_connect(SOCKINFO* info) {
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
