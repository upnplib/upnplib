/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2023-10-28
 * Cloned from pupnp ver 1.14.15.
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
// Last compare with ./pupnp source file on 2023-08-25, ver 1.14.18

#include <config.hpp>

#if EXCLUDE_MINISERVER == 0

/*!
 * \file
 *
 * \brief Implements the functionality and utility functions
 * used by the Miniserver module.
 *
 * The miniserver is a central point for processing all network requests.
 * It is made of:
 *   - The SSDP sockets for discovery.
 *   - The HTTP listeners for description / control / eventing.
 *
 */

#include <miniserver.hpp>

// #include "ThreadPool.hpp"
#include <httpreadwrite.hpp>
// #include "ithread.hpp"
#include <ssdplib.hpp>
#include <statcodes.hpp>
// #include "unixutil.hpp" /* for socklen_t, EAFNOSUPPORT */
#include <upnpapi.hpp>
// #include "upnputil.hpp"

// #include <assert.h>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <cstring>
// #include <sys/types.h>
// #include <algorithm> // for std::max()
#include <iostream>

#include <upnplib/sockaddr.hpp>
#include <upnplib/socket.hpp>
#include <upnplib/general.hpp>

#ifdef _WIN32
#include <UpnpStdInt.hpp> // for ssize_t
#endif

#include <umock/sys_socket.hpp>
#include <umock/winsock2.hpp>
#include <umock/stdlib.hpp>

/*! . */
#define APPLICATION_LISTENING_PORT 49152

struct mserv_request_t {
    /*! Connection handle. */
    SOCKET connfd;
    /*! . */
    struct sockaddr_storage foreign_sockaddr;
};

/*! . */
typedef enum {
    /*! . */
    MSERV_IDLE,
    /*! . */
    MSERV_RUNNING,
    /*! . */
    MSERV_STOPPING
} MiniServerState;

/*! . */
uint16_t miniStopSockPort;

/*!
 * module vars
 */
static MiniServerState gMServState = MSERV_IDLE;
#ifdef INTERNAL_WEB_SERVER
static MiniServerCallback gGetCallback = NULL;
static MiniServerCallback gSoapCallback = NULL;
static MiniServerCallback gGenaCallback = NULL;

static int MINISERVER_REUSEADDR =
#ifdef UPNP_MINISERVER_REUSEADDR
    1;
#else
    0;
#endif

struct s_SocketStuff {
    int ip_version;
    const char* text_addr;
    struct sockaddr_storage ss;
    union {
        struct sockaddr* serverAddr;
        struct sockaddr_in* serverAddr4;
        struct sockaddr_in6* serverAddr6;
    };
    SOCKET fd;
    uint16_t try_port;
    uint16_t actual_port;
    socklen_t address_len;
};

void SetHTTPGetCallback(MiniServerCallback callback) {
    TRACE("Executing SetHTTPGetCallback()");
    gGetCallback = callback;
}

#ifdef INCLUDE_DEVICE_APIS
void SetSoapCallback(MiniServerCallback callback) {
    TRACE("Executing SetSoapCallback()");
    gSoapCallback = callback;
}
#endif /* INCLUDE_DEVICE_APIS */

void SetGenaCallback(MiniServerCallback callback) {
    TRACE("Executing SetGenaCallback()");
    gGenaCallback = callback;
}

static int host_header_is_numeric(char* host_port, size_t host_port_size) {
    int rc = 0;
    struct in6_addr addr;
    char* s;

    /* Remove the port part. */
    s = host_port + host_port_size - 1;
    while (s != host_port && *s != ']' && *s != ':') {
        --s;
    }
    if (*s == ':') {
        *s = 0;
    } else {
        s = host_port + host_port_size;
    }

    /* Try IPV4 */
    rc = inet_pton(AF_INET, host_port, &addr);
    if (rc == 1) {
        goto ExitFunction;
    }
    /* Try IPV6 */
    /* Check for and remove the square brackets. */
    if (strlen(host_port) < 3 || host_port[0] != '[' || *(s - 1) != ']') {
        rc = 0;
        goto ExitFunction;
    }
    *(s - 1) = '\0';
    rc = inet_pton(AF_INET6, host_port + 1, &addr) == 1;

ExitFunction:
    return rc;
}

// getNumericHostRedirection() returns the ip address with port as text
// (e.g. "192.168.1.2:54321") that is bound to a socket.
static int getNumericHostRedirection(SOCKET a_socket, char* a_host_port,
                                     size_t a_hp_size) {
    TRACE("Executing getNumericHostRedirection()")
    try {
        upnplib::CSocket_basic socketObj(a_socket);
        std::string host_port = socketObj.get_addr_str();
        host_port += ':' + std::to_string(socketObj.get_port());
        memcpy(a_host_port, host_port.c_str(), a_hp_size);
        return true;

    } catch (const std::runtime_error& e) {
        std::cerr << e.what();
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what();
    }
    return false;
}

/*!
 * \brief Based on the type pf message, appropriate callback is issued.
 *
 * \return 0 on Success or HTTP_INTERNAL_SERVER_ERROR if Callback is NULL.
 */
static int dispatch_request(
    /*! [in] Socket Information object. */
    SOCKINFO* info,
    /*! [in] HTTP parser object. */
    http_parser_t* hparser) {
    memptr header;
    size_t min_size;
    http_message_t* request;
    MiniServerCallback callback;
    WebCallback_HostValidate host_validate_callback = 0;
    void* cookie{};
    int rc = UPNP_E_SUCCESS;
    /* If it does not fit in here, it is likely invalid anyway. */
    char host_port[NAME_SIZE];

    switch (hparser->msg.method) {
    /* Soap Call */
    case SOAPMETHOD_POST:
    case HTTPMETHOD_MPOST:
        callback = gSoapCallback;
        UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "miniserver %d: got SOAP msg\n", info->socket);
        break;
    /* Gena Call */
    case HTTPMETHOD_NOTIFY:
    case HTTPMETHOD_SUBSCRIBE:
    case HTTPMETHOD_UNSUBSCRIBE:
        callback = gGenaCallback;
        UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "miniserver %d: got GENA msg\n", info->socket);
        break;
    /* HTTP server call */
    case HTTPMETHOD_GET:
    case HTTPMETHOD_POST:
    case HTTPMETHOD_HEAD:
    case HTTPMETHOD_SIMPLEGET:
        callback = gGetCallback;
        host_validate_callback = gWebCallback_HostValidate;
        cookie = gWebCallback_HostValidateCookie;
        UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "miniserver %d: got WEB server msg\n", info->socket);
        break;
    default:
        callback = 0;
    }
    if (!callback) {
        rc = HTTP_INTERNAL_SERVER_ERROR;
        goto ExitFunction;
    }
    request = &hparser->msg;
#ifdef DEBUG_REDIRECT
    getNumericHostRedirection(info->socket, host_port, sizeof host_port);
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "DEBUG TEST: Redirect host_port = %s.\n", host_port);
#endif
    /* chech HOST header for an IP number -- prevents DNS rebinding. */
    if (!httpmsg_find_hdr(request, HDR_HOST, &header)) {
        rc = UPNP_E_BAD_HTTPMSG;
        goto ExitFunction;
    }
    min_size = header.length < ((sizeof host_port) - 1)
                   ? header.length
                   : (sizeof host_port) - 1;
    memcpy(host_port, header.buf, min_size);
    host_port[min_size] = 0;
    if (host_validate_callback) {
        rc = host_validate_callback(host_port, cookie);
        if (rc == UPNP_E_BAD_HTTPMSG) {
            goto ExitFunction;
        }
    } else if (!host_header_is_numeric(host_port, min_size)) {
        if (!gAllowLiteralHostRedirection) {
            rc = UPNP_E_BAD_HTTPMSG;
            UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
                       "Possible DNS Rebind attack prevented.\n");
            goto ExitFunction;
        } else {
            membuffer redir_buf;
            static const char* redir_fmt = "HTTP/1.1 307 Temporary Redirect\r\n"
                                           "Location: http://%s\r\n\r\n";
            char redir_str[NAME_SIZE];
            int timeout = HTTP_DEFAULT_TIMEOUT;

            getNumericHostRedirection(info->socket, host_port,
                                      sizeof host_port);
            membuffer_init(&redir_buf);
            snprintf(redir_str, NAME_SIZE, redir_fmt, host_port);
            membuffer_append_str(&redir_buf, redir_str);
            rc = http_SendMessage(info, &timeout, "b", redir_buf.buf,
                                  redir_buf.length);
            membuffer_destroy(&redir_buf);
            goto ExitFunction;
        }
    }
    callback(hparser, request, info);

ExitFunction:
    return rc;
}

/*!
 * \brief Send Error Message.
 */
static UPNP_INLINE void handle_error(
    /*! [in] Socket Information object. */
    SOCKINFO* info,
    /*! [in] HTTP Error Code. */
    int http_error_code,
    /*! [in] Major Version Number. */
    int major,
    /*! [in] Minor Version Number. */
    int minor) {
    http_SendStatusResponse(info, http_error_code, major, minor);
}

/*!
 * \brief Free memory assigned for handling request and unitialize socket
 * functionality.
 */
static void free_handle_request_arg(
    /*! [in] Request Message to be freed. */
    void* args) {
    TRACE("Executing free_handle_request_arg()")
    if (args == nullptr)
        return;
    struct mserv_request_t* request = (struct mserv_request_t*)args;

    sock_close(request->connfd);
    free(request);
}

/*!
 * \brief Receive the request and dispatch it for handling.
 */
static void handle_request(
    /*! [in] Request Message to be handled. */
    void* args) {
    SOCKINFO info;
    int http_error_code;
    int ret_code;
    int major = 1;
    int minor = 1;
    http_parser_t parser;
    http_message_t* hmsg = NULL;
    int timeout = HTTP_DEFAULT_TIMEOUT;
    mserv_request_t* request = (mserv_request_t*)args;
    SOCKET connfd = request->connfd;

    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver socket %d: READING\n", connfd);
    /* parser_request_init( &parser ); */ /* LEAK_FIX_MK */
    hmsg = &parser.msg;
    ret_code = sock_init_with_ip(&info, connfd,
                                 (struct sockaddr*)&request->foreign_sockaddr);
    if (ret_code != UPNP_E_SUCCESS) {
        free(request);
        httpmsg_destroy(hmsg);
        return;
    }
    /* read */
    ret_code = http_RecvMessage(&info, &parser, HTTPMETHOD_UNKNOWN, &timeout,
                                &http_error_code);
    if (ret_code != 0) {
        goto error_handler;
    }
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: PROCESSING...\n", connfd);
    /* dispatch */
    http_error_code = dispatch_request(&info, &parser);
    if (http_error_code != 0) {
        goto error_handler;
    }
    http_error_code = 0;

error_handler:
    if (http_error_code > 0) {
        if (hmsg) {
            major = hmsg->major_version;
            minor = hmsg->minor_version;
        }
        handle_error(&info, http_error_code, major, minor);
    }
    sock_destroy(&info, SD_BOTH);
    httpmsg_destroy(hmsg);
    free(request);

    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: COMPLETE\n", connfd);
}

/*!
 * \brief Initilize the thread pool to handle a request, sets priority for the
 * job and adds the job to the thread pool.
 */
static UPNP_INLINE void schedule_request_job(
    /*! [in] Socket Descriptor on which connection is accepted. */
    SOCKET connfd,
    /*! [in] Clients Address information. */
    sockaddr* clientAddr) {
    TRACE("Executing schedule_request_job()")
    UPNPLIB_LOGINFO << "MSG1042: Schedule request job to host "
                    << upnplib::to_addrport_str(
                           reinterpret_cast<const sockaddr_storage*>(
                               clientAddr))
                    << " with socket " << connfd << ".\n";
    mserv_request_t* request;
    ThreadPoolJob job{};

    request = (struct mserv_request_t*)malloc(sizeof(struct mserv_request_t));
    if (request == NULL) {
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "mserv %d: out of memory\n", connfd);
        sock_close(connfd);
        return;
    }

    request->connfd = connfd;
    memcpy(&request->foreign_sockaddr, clientAddr,
           sizeof(request->foreign_sockaddr));
    TPJobInit(&job, (start_routine)handle_request, (void*)request);
    TPJobSetFreeFunction(&job, free_handle_request_arg);
    TPJobSetPriority(&job, MED_PRIORITY);
    if (ThreadPoolAdd(&gMiniServerThreadPool, &job, NULL) != 0) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "mserv %d: cannot schedule request\n", connfd);
        free(request);
        sock_close(connfd);
        return;
    }
}
#endif // INTERNAL_WEB_SERVER

static void fdset_if_valid(SOCKET a_sock, fd_set* a_set) {
    // This sets the file descriptor set for a socket ::select() for valid
    // sochets. It ensures that ::select() does not fail with invalid socket
    // file descriiptors, in particular with the EBADF error. It checks that we
    // do not use closed or unbind sockets. It also has a guard that we do not
    // exceed the maximum number FD_SETSIZE (1024) of selectable file
    // descriptors, as noted in "man select".
    TRACE("Executing fdset_if_valid()")
    if (a_sock == INVALID_SOCKET)
        // This is a defined state and we return silently.
        return;

    if (a_sock < 3 || a_sock >= FD_SETSIZE) {
        UPNPLIB_LOGERR << "MSG1005: " << (a_sock < 0 ? "Invalid" : "Prohibited")
                       << " socket " << a_sock
                       << " not set to be monitored by ::select()"
                       << (a_sock >= 3 ? " because it violates FD_SETSIZE.\n"
                                       : ".\n");
        return;
    }
    // Check if socket is valid and bound
    try {
        upnplib::CSocket_basic sockObj(a_sock);
        if (sockObj.is_bound())

            FD_SET(a_sock, a_set);

        else
            UPNPLIB_LOGINFO << "MSG1002: Unbound socket " << a_sock
                            << " not set to be monitored by ::select().\n";

    } catch (const std::exception& e) {
        if (upnplib::g_dbug)
            std::clog << e.what();
        UPNPLIB_LOGCATCH << "MSG1009: Invalid socket " << a_sock
                         << " not set to be monitored by ::select().\n";
    }
}

static int web_server_accept([[maybe_unused]] SOCKET lsock,
                             [[maybe_unused]] fd_set& set) {
#ifndef INTERNAL_WEB_SERVER
    return UPNP_E_NO_WEB_SERVER;
#else
    TRACE("Executing web_server_accept()")
    if (lsock == INVALID_SOCKET || !FD_ISSET(lsock, &set)) {
        UPNPLIB_LOGINFO << "MSG1012: Socket(" << lsock
                        << ") invalid or not in file descriptor set.\n";
        return UPNP_E_SOCKET_ERROR;
    }

    SOCKET asock;
    socklen_t clientLen;
    sockaddr_storage clientAddr;
    sockaddr_in* sa_in{(sockaddr_in*)&clientAddr};
    clientLen = sizeof(clientAddr);

    // accept a network request connection
    asock =
        umock::sys_socket_h.accept(lsock, (sockaddr*)&clientAddr, &clientLen);
    if (asock == INVALID_SOCKET) {
        UPNPLIB_LOGERR << "MSG1022: Error in ::accept(): "
                       << std::strerror(errno) << ".\n";
        return UPNP_E_SOCKET_ACCEPT;
    }

    // Here we schedule the job to manage a UPnP request from a client.
    char buf_ntop[INET6_ADDRSTRLEN + 7];
    inet_ntop(AF_INET, &sa_in->sin_addr, buf_ntop, sizeof(buf_ntop));
    UPNPLIB_LOGINFO << "MSG1023: Connected to host " << buf_ntop << ":"
                    << ntohs(sa_in->sin_port) << " with socket " << asock
                    << ".\n";
    schedule_request_job(asock, (sockaddr*)&clientAddr);

    return UPNP_E_SUCCESS;
#endif /* INTERNAL_WEB_SERVER */
}

static void ssdp_read(SOCKET* rsock, fd_set* set) {
    TRACE("Executing ssdp_read()")
    if (*rsock == INVALID_SOCKET || !FD_ISSET(*rsock, set))
        return;

    if (readFromSSDPSocket(*rsock) != 0) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "miniserver: Error in readFromSSDPSocket(%d): "
                   "closing socket\n",
                   *rsock);
        sock_close(*rsock);
        *rsock = INVALID_SOCKET;
    }
}

static int receive_from_stopSock(SOCKET ssock, fd_set* set) {
    // The received datagram must exactly match the shutdown_str from
    // 127.0.0.1. This is a security issue to avoid that the UPnPlib can be
    // terminated from a remote ip address. Receiving 0 bytes on a datagram
    // (there's a datagram here) indicates that a zero-length datagram
    // was successful sent. This will not stop the miniserver.
    //
    // Returns 1 when the miniserver shall be stopped,
    // Returns 0 otherwise.
    TRACE("Executing receive_from_stopSock()")
    constexpr char shutdown_str[]{"ShutDown"};

    if (!FD_ISSET(ssock, set))
        return 0; // Nothing to do for this socket

    sockaddr_storage clientAddr{};
    sockaddr_in* const sa_in{reinterpret_cast<sockaddr_in*>(&clientAddr)};
    socklen_t clientLen{sizeof(clientAddr)}; // May be modified

    // The receive buffer is one byte greater with '\0' than the max receiving
    // bytes so the received message will always be terminated.
    char receiveBuf[sizeof(shutdown_str) + 1]{};
    char buf_ntop[INET6_ADDRSTRLEN];

    // receive from
    SSIZEP_T byteReceived = umock::sys_socket_h.recvfrom(
        ssock, receiveBuf, sizeof(shutdown_str), 0,
        reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
    if (byteReceived == SOCKET_ERROR ||
        inet_ntop(AF_INET, &sa_in->sin_addr, buf_ntop, sizeof(buf_ntop)) ==
            nullptr) {
        UPNPLIB_LOGCRIT << "MSG1038: Failed to receive data from socket "
                        << ssock << ". Stop miniserver.\n";
        return 1;
    }

    // 16777343 are netorder bytes of "127.0.0.1"
    if (sa_in->sin_addr.s_addr != 16777343 ||
        strcmp(receiveBuf, shutdown_str) != 0) //
    {
        char nullstr[]{"\\0"};
        if (byteReceived == 0 || receiveBuf[byteReceived - 1] != '\0')
            nullstr[0] = '\0';
        UPNPLIB_LOGERR << "MSG1039: Received \"" << receiveBuf << nullstr
                       << "\" from " << buf_ntop << ":"
                       << ntohs(sa_in->sin_port)
                       << ", must be \"ShutDown\\0\" from 127.0.0.1:*. Don't "
                          "stopping miniserver.\n";
        return 0;
    }

    UPNPLIB_LOGINFO << "MSG1040: Received ordinary datagram \"" << receiveBuf
                    << "\\0\" from " << buf_ntop << ":"
                    << ntohs(sa_in->sin_port) << ". Stop miniserver.\n";
    return 1;
}

/*!
 * \brief Run the miniserver.
 *
 * The MiniServer accepts a new request and schedules a thread to handle the
 * new request. Checks for socket state and invokes appropriate read and
 * shutdown actions for the Miniserver and SSDP sockets.
 */
static void RunMiniServer(
    /*! [in] Socket Array. */
    MiniServerSockArray* miniSock) {
    TRACE("Executing RunMiniServer()")
    fd_set expSet;
    fd_set rdSet;
    int stopSock = 0;

    // On MS Windows INVALID_SOCKET is unsigned -1 = 18446744073709551615 so we
    // get maxMiniSock with this big number even if there is only one
    // INVALID_SOCKET. Incrementing it at the end results in 0. To be portable
    // we must not assume INVALID_SOCKET to be -1. --Ingo
    SOCKET maxMiniSock = 0;
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->miniServerSock4 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->miniServerSock4);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->miniServerSock6 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->miniServerSock6);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->miniServerSock6UlaGua == INVALID_SOCKET
                                  ? 0
                                  : miniSock->miniServerSock6UlaGua);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->miniServerStopSock == INVALID_SOCKET
                                  ? 0
                                  : miniSock->miniServerStopSock);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpSock4 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpSock4);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpSock6 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpSock6);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpSock6UlaGua == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpSock6UlaGua);
#ifdef INCLUDE_CLIENT_APIS
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpReqSock4 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpReqSock4);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpReqSock6 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpReqSock6);
#endif /* INCLUDE_CLIENT_APIS */
    ++maxMiniSock;

    gMServState = MSERV_RUNNING;
    while (!stopSock) {
        FD_ZERO(&rdSet);
        FD_ZERO(&expSet);
        /* FD_SET()'s */
        FD_SET(miniSock->miniServerStopSock, &expSet);
        FD_SET(miniSock->miniServerStopSock, &rdSet);
        fdset_if_valid(miniSock->miniServerSock4, &rdSet);
        fdset_if_valid(miniSock->miniServerSock6, &rdSet);
        fdset_if_valid(miniSock->miniServerSock6UlaGua, &rdSet);
        fdset_if_valid(miniSock->ssdpSock4, &rdSet);
        fdset_if_valid(miniSock->ssdpSock6, &rdSet);
        fdset_if_valid(miniSock->ssdpSock6UlaGua, &rdSet);
#ifdef INCLUDE_CLIENT_APIS
        fdset_if_valid(miniSock->ssdpReqSock4, &rdSet);
        fdset_if_valid(miniSock->ssdpReqSock6, &rdSet);
#endif /* INCLUDE_CLIENT_APIS */

        /* select() */
        int ret = umock::sys_socket_h.select(static_cast<int>(maxMiniSock),
                                             &rdSet, NULL, &expSet, NULL);

        if (ret == SOCKET_ERROR) {
            if (errno == EINTR) {
                // A signal was caught, not for us. We ignore it and
                continue;
            }
            if (errno == EBADF) {
                // A closed socket file descriptor was given in one of the
                // sets. For details look at
                // REF: [Should I assert fail on select() EBADF?]
                // (https://stackoverflow.com/q/28015859/5014688)
                // It is difficult to determine here what file descriptor
                // in rdSet or expSet is invalid. So I ensure that only valid
                // socket fds are given by checking them with fdset_if_valid()
                // before calling select(). Doing this I have to
                continue;
            }
            // All other errors EINVAL and ENOMEM are critical and cannot
            // continue run mininserver.
            UPNPLIB_LOGCRIT "MSG1021: Error in ::select(): "
                << std::strerror(errno) << ".\n";
            break;
        }

        // Accept requested connection from a remote client and run the
        // connection in a new thread. Due to side effects we need to
        // avoid lazy evaluation with chained ||.
        [[maybe_unused]] int ret1 =
            web_server_accept(miniSock->miniServerSock4, rdSet);
        [[maybe_unused]] int ret2 =
            web_server_accept(miniSock->miniServerSock6, rdSet);
        [[maybe_unused]] int ret3 =
            web_server_accept(miniSock->miniServerSock6UlaGua, rdSet);
        // if (ret1 == UPNP_E_SUCCESS || ret2 == UPNP_E_SUCCESS ||
        //     ret3 == UPNP_E_SUCCESS) {
#ifdef INCLUDE_CLIENT_APIS
        ssdp_read(&miniSock->ssdpReqSock4, &rdSet);
        ssdp_read(&miniSock->ssdpReqSock6, &rdSet);
#endif /* INCLUDE_CLIENT_APIS */
        ssdp_read(&miniSock->ssdpSock4, &rdSet);
        ssdp_read(&miniSock->ssdpSock6, &rdSet);
        ssdp_read(&miniSock->ssdpSock6UlaGua, &rdSet);
        // }

        // Check if we have received a packet from
        // localhost(127.0.0.1) that will stop the miniserver.
        stopSock = receive_from_stopSock(miniSock->miniServerStopSock, &rdSet);
    } // while (!stopsock)

    /* Close all sockets. */
    sock_close(miniSock->miniServerSock4);
    sock_close(miniSock->miniServerSock6);
    sock_close(miniSock->miniServerSock6UlaGua);
    sock_close(miniSock->miniServerStopSock);
    sock_close(miniSock->ssdpSock4);
    sock_close(miniSock->ssdpSock6);
    sock_close(miniSock->ssdpSock6UlaGua);
#ifdef INCLUDE_CLIENT_APIS
    sock_close(miniSock->ssdpReqSock4);
    sock_close(miniSock->ssdpReqSock6);
#endif /* INCLUDE_CLIENT_APIS */
    /* Free minisock. */
    umock::stdlib_h.free(miniSock);
    gMServState = MSERV_IDLE;

    return;
}

/*!
 * \brief Returns port to which socket, sockfd, is bound.
 *
 * \return -1 on error; check errno. 0 if successful.
 */
static int get_port(
    /*! [in] Socket descriptor. */
    SOCKET sockfd,
    /*! [out] The port value if successful, otherwise, untouched. */
    uint16_t* port) {
    TRACE("Executing get_port(), calls system getsockname()")
    sockaddr_storage sockinfo{};
    socklen_t len(sizeof sockinfo); // May be modified by getsockname()

    if (umock::sys_socket_h.getsockname(sockfd, (sockaddr*)&sockinfo, &len) ==
        -1)
        return -1;

    switch (sockinfo.ss_family) {
    case AF_INET:
        *port = ntohs(((sockaddr_in*)&sockinfo)->sin_port);
        break;
    case AF_INET6:
        *port = ntohs(((sockaddr_in6*)&sockinfo)->sin6_port);
        break;
    default:
        return -1;
    }
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "sockfd = %d, .... port = %d\n", sockfd, (int)*port);

    return 0;
}

/*!
 * \brief Get valid sockets.
 *
 * An empty text_addr will be translated to a valid sock addr = 0 that
 * binds to all local ip addresses.
 * \return 1 or WSANOTINITIALISED on error, 0 if successful.
 */
#ifdef INTERNAL_WEB_SERVER
static int init_socket_suff(struct s_SocketStuff* s, const char* text_addr,
                            int ip_version) {
    TRACE("Executing init_socket_suff()")
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "Inside init_socket_suff() for IPv%d.\n", ip_version);

    int sockError;
    sa_family_t domain;
    void* addr; // This holds a pointer to sin_addr, not a value
    int reuseaddr_on = MINISERVER_REUSEADDR;

    memset(s, 0, sizeof *s);
    s->fd = INVALID_SOCKET;
    s->ip_version = ip_version;
    s->text_addr = text_addr;
    s->serverAddr = (struct sockaddr*)&s->ss;
    switch (ip_version) {
    case 4:
        domain = AF_INET;
        s->serverAddr4->sin_family = domain;
        s->address_len = sizeof *s->serverAddr4;
        addr = &s->serverAddr4->sin_addr;
        break;
    case 6:
        domain = AF_INET6;
        s->serverAddr6->sin6_family = domain;
        s->address_len = sizeof *s->serverAddr6;
        addr = &s->serverAddr6->sin6_addr;
        break;
    default:
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "init_socket_suff(): Invalid IP version: %d.\n", ip_version);
        goto error;
        break;
    }

    if (inet_pton(domain, text_addr, addr) <= 0) {
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "init_socket_suff(): Invalid ip address: %s.\n", text_addr);
        goto error;
    }
    s->fd = umock::sys_socket_h.socket(domain, SOCK_STREAM, 0);

    if (s->fd == INVALID_SOCKET) {
#ifdef _WIN32
        int errval = umock::winsock2_h.WSAGetLastError();
        if (errval == WSANOTINITIALISED)
            UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                       "init_socket_suff(): WSAStartup() wasn't called to "
                       "initialize use of sockets\n");
        else
            UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                       "init_socket_suff(): WSAGetLastError() returned %d.\n",
                       errval);
#else
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "init_socket_suff(): IPv%d socket not available: "
                   "%s\n",
                   ip_version, std::strerror(errno));
#endif
        goto error;
    } else if (ip_version == 6) {
        int onOff = 1;

        sockError = umock::sys_socket_h.setsockopt(
            s->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&onOff, sizeof(onOff));
        if (sockError == SOCKET_ERROR) {
            UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                       "init_socket_suff(): unable to set IPv6 "
                       "socket protocol: %s\n",
                       std::strerror(errno));
            goto error;
        }
    }
    /* Getting away with implementation of re-using address:port and
     * instead choosing to increment port numbers.
     * Keeping the re-use address code as an optional behaviour that
     * can be turned on if necessary.
     * TURN ON the reuseaddr_on option to use the option. */
    if (MINISERVER_REUSEADDR) {
        sockError = umock::sys_socket_h.setsockopt(
            s->fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr_on,
            sizeof(int));
        if (sockError == SOCKET_ERROR) {
            UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                       "init_socket_suff(): unable to set "
                       "SO_REUSEADDR: %s\n",
                       std::strerror(errno));
            goto error;
        }
    }

    return 0;

error:
    if (s->fd != INVALID_SOCKET) {
        sock_close(s->fd);
    }
    s->fd = INVALID_SOCKET;

    // return errval; // errval is available here but to be compatible I use
    return 1;
}

/*
 * s->port will be one more than the used port in the end. This is important,
 * in case this function is called again.
 * It is expected to have a prechecked valid parameter.
 */
static int do_bind(s_SocketStuff* s) {
    TRACE("Executing do_bind()")
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__, "Inside do_bind()\n");

    int ret_val = UPNP_E_SUCCESS;
    int bind_error;
    uint16_t original_listen_port = s->try_port;

    bool repeat_do{false};
    do {
        switch (s->ip_version) {
        case 4:
            // Compilation Error on macOS:
            // operation on 's->s_SocketStuff::try_port' may be undefined
            // [-Werror=sequence-point] s->serverAddr4->sin_port =
            // htons(s->try_port++); --Ingo
            s->serverAddr4->sin_port = htons(s->try_port);
            s->actual_port = s->try_port;
            s->try_port = s->try_port + 1;
            break;
        case 6:
            // Compilation Error on macOS:
            // operation on 's->s_SocketStuff::try_port' may be undefined
            // [-Werror=sequence-point] s->serverAddr6->sin6_port =
            // htons(s->try_port++); --Ingo
            s->serverAddr6->sin6_port = htons(s->try_port);
            s->actual_port = s->try_port;
            s->try_port = s->try_port + 1;
            break;
        default:
            ret_val = UPNP_E_INVALID_PARAM;
            goto error;
        }
        if (s->try_port == 0)
            s->try_port = APPLICATION_LISTENING_PORT;

        // Bind socket
        bind_error =
            umock::sys_socket_h.bind(s->fd, s->serverAddr, s->address_len);

        if (bind_error != 0)
#ifdef _MSC_VER
            repeat_do = (umock::winsock2_h.WSAGetLastError() == WSAEADDRINUSE)
                            ? true
                            : false;
#else
            repeat_do = (errno == EADDRINUSE) ? true : false;
#endif
    } while (repeat_do && s->try_port >= original_listen_port);

    if (bind_error != 0) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "do_bind(): "
#ifdef _MSC_VER
                   "Error with IPv%d returned from ::bind() = %d.\n",
                   s->ip_version, umock::winsock2_h.WSAGetLastError()
#else
                   "Error with IPv%d returned from ::bind() = %s.\n",
                   s->ip_version, std::strerror(errno)
#endif
        );
        /* Bind failed. */
        ret_val = UPNP_E_SOCKET_BIND;
        goto error;
    }

    return UPNP_E_SUCCESS;

error:
    return ret_val;
}

static int do_listen(struct s_SocketStuff* s) {
    TRACE("Executing do_listen()")
    int ret_val;
    int listen_error;
    int port_error;

    listen_error = umock::sys_socket_h.listen(s->fd, SOMAXCONN);
    if (listen_error == -1) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "do_listen(): Error in IPv%d listen(): %s.\n", s->ip_version,
                   std::strerror(errno));
        ret_val = UPNP_E_LISTEN;
        goto error;
    }
    port_error = get_port(s->fd, &s->actual_port);
    if (port_error < 0) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "do_listen(): Error in IPv%d get_port(): %s.\n",
                   s->ip_version, std::strerror(errno));
        ret_val = UPNP_E_INTERNAL_ERROR;
        goto error;
    }

    return UPNP_E_SUCCESS;

error:
    return ret_val;
}

static int do_reinit(struct s_SocketStuff* s) {
    TRACE("Executing do_reinit()");
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__, "Inside do_reinit()\n");

    sock_close(s->fd);

    return init_socket_suff(s, s->text_addr, s->ip_version);
}

static int do_bind_listen(s_SocketStuff* s) {
    TRACE("Executing do_bind_listen()")
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "Inside do_bind_listen()\n");

    int ret_val;
    int ok = 0;
    uint16_t original_port = s->try_port;

    while (!ok) {
        ret_val = do_bind(s);
        if (ret_val) {
            goto error;
        }
        ret_val = do_listen(s);
        if (ret_val) {
            if (errno == EADDRINUSE) {
                do_reinit(s);
                s->try_port = original_port;
                continue;
            }
            goto error;
        }
        ok = s->try_port >= original_port;
    }

    return 0;

error:
    return ret_val;
}

/*!
 * \brief Creates a STREAM socket, binds to INADDR_ANY and listens for
 * incoming connecttions. Returns the actual port which the sockets
 * sub-system returned.
 *
 * Also creates a DGRAM socket, binds to the loop back address and
 * returns the port allocated by the socket sub-system.
 *
 * \return
 *  \li UPNP_E_OUTOF_SOCKET: Failed to create a socket.
 *  \li UPNP_E_SOCKET_BIND: Bind() failed.
 *  \li UPNP_E_LISTEN: Listen() failed.
 *  \li UPNP_E_INTERNAL_ERROR: Port returned by the socket layer is < 0.
 *  \li UPNP_E_SUCCESS: Success.
 */
static int get_miniserver_sockets(
    /*! [in] Socket Array. */
    MiniServerSockArray* out,
    /*! [in] port on which the server is listening for incoming IPv4
     * connections. */
    uint16_t listen_port4,
    /*! [in] port on which the server is listening for incoming IPv6
     * ULA connections. */
    uint16_t listen_port6,
    /*! [in] port on which the server is listening for incoming
     * IPv6 ULA or GUA connections. */
    uint16_t listen_port6UlaGua) {
    TRACE("Executing get_miniserver_sockets()")
    int ret_val{UPNP_E_INTERNAL_ERROR};
    int err_init_4;
    int err_init_6;
    int err_init_6UlaGua;

    s_SocketStuff ss4;
    s_SocketStuff ss6;
    s_SocketStuff ss6UlaGua;

    /* Create listen socket for IPv4/IPv6. An error here may indicate
     * that we don't have an IPv4/IPv6 stack. */
    err_init_4 = init_socket_suff(&ss4, gIF_IPV4, 4);
    err_init_6 = init_socket_suff(&ss6, gIF_IPV6, 6);
    err_init_6UlaGua = init_socket_suff(&ss6UlaGua, gIF_IPV6_ULA_GUA, 6);
    ss6.serverAddr6->sin6_scope_id = gIF_INDEX;
    /* Check what happened. */
#ifdef _WIN32
    if (err_init_4 == WSANOTINITIALISED) {
        ret_val = UPNP_E_INIT_FAILED;
        goto error;
    }
#endif
    if (err_init_4 && (err_init_6 || err_init_6UlaGua)) {
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "get_miniserver_sockets: no protocols available\n");
        ret_val = UPNP_E_OUTOF_SOCKET;
        goto error;
    }
    /* As per the IANA specifications for the use of ports by applications
     * override the listen port passed in with the first available. */
    if (listen_port4 == 0) {
        listen_port4 = (uint16_t)APPLICATION_LISTENING_PORT;
    }
    if (listen_port6 == 0) {
        listen_port6 = (uint16_t)APPLICATION_LISTENING_PORT;
    }
    if (listen_port6UlaGua < APPLICATION_LISTENING_PORT) {
        /* Increment the port to make it harder to fail at first try */
        listen_port6UlaGua = listen_port6 + 1;
    }
    ss4.try_port = listen_port4;
    ss6.try_port = listen_port6;
    ss6UlaGua.try_port = listen_port6UlaGua;
    if (MINISERVER_REUSEADDR) {
        /* THIS IS ALLOWS US TO BIND AGAIN IMMEDIATELY
         * AFTER OUR SERVER HAS BEEN CLOSED
         * THIS MAY CAUSE TCP TO BECOME LESS RELIABLE
         * HOWEVER IT HAS BEEN SUGESTED FOR TCP SERVERS. */
        UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "get_miniserver_sockets: resuseaddr is set.\n");
    }
    if (ss4.fd != INVALID_SOCKET) {
        ret_val = do_bind_listen(&ss4);
        if (ret_val) {
            goto error;
        }
    }
    if (ss6.fd != INVALID_SOCKET) {
        ret_val = do_bind_listen(&ss6);
        if (ret_val) {
            goto error;
        }
    }
    if (ss6UlaGua.fd != INVALID_SOCKET) {
        ret_val = do_bind_listen(&ss6UlaGua);
        if (ret_val) {
            goto error;
        }
    }
    // BUG! the following condition may be wrong, e.g. with ss4.fd ==
    // INVALID_SOCKET and ENABLE_IPV6 disabled but also others. --Ingo
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               // "get_miniserver_sockets: bind successful\n");
               "get_miniserver_sockets: finished\n");
    out->miniServerPort4 = ss4.actual_port;
    out->miniServerPort6 = ss6.actual_port;
    out->miniServerPort6UlaGua = ss6UlaGua.actual_port;
    out->miniServerSock4 = ss4.fd;
    out->miniServerSock6 = ss6.fd;
    out->miniServerSock6UlaGua = ss6UlaGua.fd;

    return UPNP_E_SUCCESS;

error:
    if (ss4.fd != INVALID_SOCKET) {
        sock_close(ss4.fd);
    }
    if (ss6.fd != INVALID_SOCKET) {
        sock_close(ss6.fd);
    }
    if (ss6UlaGua.fd != INVALID_SOCKET) {
        sock_close(ss6UlaGua.fd);
    }

    return ret_val;
}
#endif /* INTERNAL_WEB_SERVER */

/*!
 * \brief Creates the miniserver STOP socket. This socket is created and
 *  listened on to know when it is time to stop the Miniserver.
 *
 * \return
 * \li \c UPNP_E_OUTOF_SOCKET: Failed to create a socket.
 * \li \c UPNP_E_SOCKET_BIND: Bind() failed.
 * \li \c UPNP_E_INTERNAL_ERROR: Port returned by the socket layer is < 0.
 * \li \c UPNP_E_SUCCESS: Success.
 */
static int get_miniserver_stopsock(
    /*! [in] Miniserver Socket Array. */
    MiniServerSockArray* out) {
    TRACE("Executing get_miniserver_stopsock()")
    struct sockaddr_in stop_sockaddr;
    SOCKET miniServerStopSock = 0;
    int ret = 0;

    miniServerStopSock = umock::sys_socket_h.socket(AF_INET, SOCK_DGRAM, 0);
    if (miniServerStopSock == INVALID_SOCKET) {
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "Error in socket(): %s\n", std::strerror(errno));
        return UPNP_E_OUTOF_SOCKET;
    }
    /* Bind to local socket. */
    memset(&stop_sockaddr, 0, sizeof(stop_sockaddr));
    stop_sockaddr.sin_family = (sa_family_t)AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &stop_sockaddr.sin_addr);
    ret = umock::sys_socket_h.bind(miniServerStopSock,
                                   (struct sockaddr*)&stop_sockaddr,
                                   sizeof(stop_sockaddr));
    if (ret == SOCKET_ERROR) {
        UpnpPrintf(UPNP_CRITICAL, MSERV, __FILE__, __LINE__,
                   "Error in binding localhost: %s.\n,", std::strerror(errno));
        sock_close(miniServerStopSock);
        return UPNP_E_SOCKET_BIND;
    }
    ret = get_port(miniServerStopSock, &miniStopSockPort);
    if (ret < 0) {
        sock_close(miniServerStopSock);
        return UPNP_E_INTERNAL_ERROR;
    }
    out->miniServerStopSock = miniServerStopSock;
    out->stopPort = miniStopSockPort;

    return UPNP_E_SUCCESS;
}

static void InitMiniServerSockArray(MiniServerSockArray* miniSocket) {
    TRACE("Executing InitMiniServerSockArray()")
    miniSocket->miniServerSock4 = INVALID_SOCKET;
    miniSocket->miniServerSock6 = INVALID_SOCKET;
    miniSocket->miniServerSock6UlaGua = INVALID_SOCKET;
    miniSocket->miniServerStopSock = INVALID_SOCKET;
    miniSocket->ssdpSock4 = INVALID_SOCKET;
    miniSocket->ssdpSock6 = INVALID_SOCKET;
    miniSocket->ssdpSock6UlaGua = INVALID_SOCKET;
    miniSocket->stopPort = 0u;
    miniSocket->miniServerPort4 = 0u;
    miniSocket->miniServerPort6 = 0u;
    miniSocket->miniServerPort6UlaGua = 0u;
#ifdef INCLUDE_CLIENT_APIS
    miniSocket->ssdpReqSock4 = INVALID_SOCKET;
    miniSocket->ssdpReqSock6 = INVALID_SOCKET;
#endif /* INCLUDE_CLIENT_APIS */
}

int StartMiniServer(
    // The three parameter only used if the INTERNAL_WEB_SERVER is enabled.
    // The miniserver does not need them.
    //
    /*! [in,out] Port on which the server listens for incoming IPv4
     * connections. */
    [[maybe_unused]] uint16_t* listen_port4,
    /*! [in,out] Port on which the server listens for incoming IPv6
     * LLA connections. */
    [[maybe_unused]] uint16_t* listen_port6,
    /*! [in,out] Port on which the server listens for incoming
     * IPv6 ULA or GUA connections. */
    [[maybe_unused]] uint16_t* listen_port6UlaGua) {
    TRACE("Executing StartMiniServer()")
    int ret_code;
    int count;
    int max_count = 10000;
    MiniServerSockArray* miniSocket;
    ThreadPoolJob job;

    memset(&job, 0, sizeof(job));

    switch (gMServState) {
    case MSERV_IDLE:
        break;
    default:
        /* miniserver running. */
        return UPNP_E_INTERNAL_ERROR;
    }
    miniSocket = (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
    if (!miniSocket) {
        return UPNP_E_OUTOF_MEMORY;
    }
    InitMiniServerSockArray(miniSocket);
#ifdef INTERNAL_WEB_SERVER
    /* V4 and V6 http listeners. */
    ret_code = get_miniserver_sockets(miniSocket, *listen_port4, *listen_port6,
                                      *listen_port6UlaGua);
    if (ret_code != UPNP_E_SUCCESS) {
        free(miniSocket);
        return ret_code;
    }
#endif
    /* Stop socket (To end miniserver processing). */
    ret_code = get_miniserver_stopsock(miniSocket);
    if (ret_code != UPNP_E_SUCCESS) {
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        free(miniSocket);
        return ret_code;
    }
    /* SSDP socket for discovery/advertising. */
    ret_code = get_ssdp_sockets(miniSocket);
    if (ret_code != UPNP_E_SUCCESS) {
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        sock_close(miniSocket->miniServerStopSock);
        free(miniSocket);
        return ret_code;
    }
    TPJobInit(&job, (start_routine)RunMiniServer, (void*)miniSocket);
    TPJobSetPriority(&job, MED_PRIORITY);
    TPJobSetFreeFunction(&job, (free_routine)free);
    ret_code = ThreadPoolAddPersistent(&gMiniServerThreadPool, &job, NULL);
    if (ret_code < 0) {
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        sock_close(miniSocket->miniServerStopSock);
        sock_close(miniSocket->ssdpSock4);
        sock_close(miniSocket->ssdpSock6);
        sock_close(miniSocket->ssdpSock6UlaGua);
#ifdef INCLUDE_CLIENT_APIS
        sock_close(miniSocket->ssdpReqSock4);
        sock_close(miniSocket->ssdpReqSock6);
#endif /* INCLUDE_CLIENT_APIS */
        free(miniSocket);
        return UPNP_E_OUTOF_MEMORY;
    }
    /* Wait for miniserver to start. */
    count = 0;
    while (gMServState != (MiniServerState)MSERV_RUNNING && count < max_count) {
        /* 0.05s */
        imillisleep(50);
        count++;
    }
    if (count >= max_count) {
        /* Took it too long to start that thread. */
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        sock_close(miniSocket->miniServerStopSock);
        sock_close(miniSocket->ssdpSock4);
        sock_close(miniSocket->ssdpSock6);
        sock_close(miniSocket->ssdpSock6UlaGua);
#ifdef INCLUDE_CLIENT_APIS
        sock_close(miniSocket->ssdpReqSock4);
        sock_close(miniSocket->ssdpReqSock6);
#endif /* INCLUDE_CLIENT_APIS */
        return UPNP_E_INTERNAL_ERROR;
    }
#ifdef INTERNAL_WEB_SERVER
    *listen_port4 = miniSocket->miniServerPort4;
    *listen_port6 = miniSocket->miniServerPort6;
    *listen_port6UlaGua = miniSocket->miniServerPort6UlaGua;
#endif

    return UPNP_E_SUCCESS;
}

int StopMiniServer() {
    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "Inside StopMiniServer()\n");

    socklen_t socklen = sizeof(struct sockaddr_in);
    SOCKET sock;
    struct sockaddr_in ssdpAddr;
    char buf[256] = "ShutDown";
    // due to required type cast for 'sendto' on WIN32 bufLen must fit to an int
    size_t bufLen = strlen(buf);

    switch (gMServState) {
    case MSERV_RUNNING:
        gMServState = MSERV_STOPPING;
        break;
    default:
        return 0;
    }
    sock = umock::sys_socket_h.socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        UpnpPrintf(UPNP_ERROR, SSDP, __FILE__, __LINE__,
                   "SSDP_SERVER: StopSSDPServer: Error in socket() %s\n",
                   std::strerror(errno));
        return 0;
    }
    while (gMServState != (MiniServerState)MSERV_IDLE) {
        ssdpAddr.sin_family = (sa_family_t)AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &ssdpAddr.sin_addr);
        ssdpAddr.sin_port = htons(miniStopSockPort);
        umock::sys_socket_h.sendto(sock, buf, (SIZEP_T)bufLen, 0,
                                   (struct sockaddr*)&ssdpAddr, socklen);
        imillisleep(1);
        if (gMServState == (MiniServerState)MSERV_IDLE) {
            break;
        }
        isleep(1u);
    }
    sock_close(sock);

    return 0;
}
#endif /* EXCLUDE_MINISERVER */
