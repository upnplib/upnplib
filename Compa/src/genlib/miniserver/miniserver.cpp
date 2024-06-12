/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-06-13
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
/*!
 * \file
 * \ingroup compa-Addressing
 * \brief Implements the functionality and utility functions used by the
 * Miniserver module.
 *
 * The miniserver is a central point for processing all network requests.
 * It is made of:
 *   - The HTTP listeners for description / control / eventing.
 *   - The SSDP sockets for discovery.
 */

#include <miniserver.hpp>

#include <httpreadwrite.hpp>
#include <ssdp_common.hpp>
#include <statcodes.hpp>
#include <upnpapi.hpp>

#include <upnplib/socket.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>

#include <umock/sys_socket.hpp>
#include <umock/winsock2.hpp>
#include <umock/stdlib.hpp>
#include <umock/pupnp_miniserver.hpp>
#include <umock/pupnp_ssdp.hpp>


/// \cond
#include <cstring>
#include <random>
/// \endcond

namespace {

#ifdef COMPA_HAVE_WEBSERVER
/*! \brief First dynamic and/or private port from 49152 to 65535 used by the
 * library.\n
 * Only available with webserver compiled in. */
constexpr in_port_t APPLICATION_LISTENING_PORT{49152};
#endif

/*! \brief miniserver received request message.
 * \details This defines the structure of a UPnP request that has income from a
 * remote control point.
 */
struct mserv_request_t {
    /// \brief Connection socket file descriptor.
    SOCKET connfd;
    /// \brief Socket address of the remote control point.
    sockaddr_storage foreign_sockaddr;
};

/// \brief miniserver state
enum MiniServerState {
    MSERV_IDLE,    ///< miniserver is idle.
    MSERV_RUNNING, ///< miniserver is running.
    MSERV_STOPPING ///< miniserver is running to stop.
};

/*! \brief Port of the stop socket.
 *  \details With starting the miniserver there is also this port registered.
 * Its socket is listing for a "ShutDown" message from a local network address
 * (localhost). This is used to stop the miniserver from another thread.
 */
in_port_t miniStopSockPort;

/// \brief miniserver state
MiniServerState gMServState{MSERV_IDLE};
#ifdef COMPA_HAVE_WEBSERVER
/// \brief SOAP callback
MiniServerCallback gSoapCallback{nullptr};
/// \brief GENA callback
MiniServerCallback gGenaCallback{nullptr};

/*! \name Scope restricted to file
 * @{ */

/*!
 * \brief Check if a network address is numeric.
 *
 * An empty netaddress or an unspecified one ("[::]", "0.0.0.0") is not valid.
 */
// No unit test needed. It's tested with SSockaddr.
int host_header_is_numeric(
    char* a_host_port,     ///< network address
    size_t a_host_port_len ///< length of a_host_port excl. terminating '\0'.
) {
    TRACE("Executing host_header_is_numeric()");
    if (a_host_port_len == 0 || strncmp(a_host_port, "[::]", 4) == 0 ||
        strncmp(a_host_port, "0.0.0.0", 7) == 0)
        return 0;

    upnplib::SSockaddr saddrObj;
    try {
        saddrObj = std::string(a_host_port, a_host_port_len);
    } catch (const std::exception& e) {
        UPNPLIB_LOGCATCH "MSG1049: " << e.what() << "\n";
        return 0;
    }
    return 1;
}

/*! \brief Returns the ip address with port as text that is bound to a socket.
 *
 * Example: may return "[2001:db8::ab]:50044" or "192.168.1.2:54321".
 *
 * \returns
 *   On success: **true**\n
 *   On error: **false**, The result buffer remains unmodified.
 */
int getNumericHostRedirection(
    SOCKET a_socket,   ///< [in] Socket file descriptor.
    char* a_host_port, ///< [out] Pointer to buffer that will be filled.
    size_t a_hp_size   ///< [in] size of the buffer.
) {
    TRACE("Executing getNumericHostRedirection()")
    try {
        upnplib::CSocket_basic socketObj(a_socket);
        socketObj.init();
        std::string host_port = socketObj.netaddrp();
        memcpy(a_host_port, host_port.c_str(), a_hp_size);
        return true;

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return false;
}

/*!
 * \brief Based on the type of message, appropriate callback is issued.
 *
 * \returns
 *  On success: **0**\n
 *  On error: HTTP_INTERNAL_SERVER_ERROR if Callback is NULL.
 */
int dispatch_request(
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
        UPNPLIB_LOGINFO "MSG1107: miniserver socket="
            << info->socket << ": got WEB server msg.\n";
        break;
    default:
        callback = 0;
    }
    if (!callback) {
        rc = HTTP_INTERNAL_SERVER_ERROR;
        goto ExitFunction;
    }
    request = &hparser->msg;
    if (upnplib::g_dbug) {
        getNumericHostRedirection(info->socket, host_port, sizeof host_port);
        UPNPLIB_LOGINFO "MSG1113: Redirect host_port=\"" << host_port << "\"\n";
    }
    /* check HOST header for an IP number -- prevents DNS rebinding. */
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
 * \brief Free memory assigned for handling request and unitialize socket
 * functionality.
 */
void free_handle_request_arg(
    /*! [in] Request Message to be freed. */
    void* args) {
    TRACE("Executing free_handle_request_arg()")
    if (args == nullptr)
        return;

    sock_close(static_cast<mserv_request_t*>(args)->connfd);
    free(args);
}

/*!
 * \brief Receive the request and dispatch it for handling.
 */
void handle_request(
    /*! [in] Received Request Message to be handled. */
    void* args) { // Expected to be mserv_request_t*
    SOCKINFO info;
    int http_error_code;
    int ret_code;
    int major = 1;
    int minor = 1;
    http_parser_t parser;
    http_message_t* hmsg = NULL;
    int timeout = HTTP_DEFAULT_TIMEOUT;
    mserv_request_t* request_in = (mserv_request_t*)args;
    SOCKET connfd = request_in->connfd;

    UPNPLIB_LOGINFO "MSG1027: Miniserver socket "
        << connfd << ": READING request from client...\n";
    /* parser_request_init( &parser ); */ /* LEAK_FIX_MK */
    hmsg = &parser.msg;
    ret_code = sock_init_with_ip(&info, connfd,
                                 (sockaddr*)&request_in->foreign_sockaddr);
    if (ret_code != UPNP_E_SUCCESS) {
        free(request_in);
        httpmsg_destroy(hmsg);
        return;
    }

    /* read */
    ret_code = http_RecvMessage(&info, &parser, HTTPMETHOD_UNKNOWN, &timeout,
                                &http_error_code);
    if (ret_code != 0) {
        goto error_handler;
    }

    UPNPLIB_LOGINFO "MSG1106: miniserver socket=" << connfd
                                                  << ": PROCESSING...\n";
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
        // BUG! Don't try to send a status response to a remote client with
        // http error (e.g. 400) if we have a socket error. It doesn't make
        // sense. --Ingo
        http_SendStatusResponse(&info, http_error_code, major, minor);
    }
    sock_destroy(&info, SD_BOTH);
    httpmsg_destroy(hmsg);
    free(request_in);

    UpnpPrintf(UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: COMPLETE\n", connfd);
}

/*!
 * \brief Initilize the thread pool to handle a request, sets priority for the
 * job and adds the job to the thread pool.
 */
UPNP_INLINE void schedule_request_job(
    /*! [in] Socket Descriptor on which connection is accepted. */
    SOCKET connfd,
    /*! [in] Clients Address information. */
    sockaddr* clientAddr) {
    TRACE("Executing schedule_request_job()")
    UPNPLIB_LOGINFO "MSG1042: Schedule request job to host "
        << upnplib::to_netaddrp(
               reinterpret_cast<const sockaddr_storage*>(clientAddr))
        << " with socket " << connfd << ".\n";

    ThreadPoolJob job{};
    mserv_request_t* request{
        static_cast<mserv_request_t*>(std::malloc(sizeof(mserv_request_t)))};

    if (request == nullptr) {
        UPNPLIB_LOGCRIT "MSG1024: Socket " << connfd << ": out of memory.\n";
        sock_close(connfd);
        return;
    }

    request->connfd = connfd;
    memcpy(&request->foreign_sockaddr, clientAddr,
           sizeof(request->foreign_sockaddr));
    TPJobInit(&job, (start_routine)handle_request, request);
    TPJobSetFreeFunction(&job, free_handle_request_arg);
    TPJobSetPriority(&job, MED_PRIORITY);
    if (ThreadPoolAdd(&gMiniServerThreadPool, &job, NULL) != 0) {
        UPNPLIB_LOGERR "MSG1025: Socket " << connfd
                                          << ": cannot schedule request.\n";
        free(request);
        sock_close(connfd);
        return;
    }
}
#endif // COMPA_HAVE_WEBSERVER

/*!
 * \brief Add a socket file descriptor to an \p 'fd_set' structure as needed for
 * \p \::select().
 *
 * **a_set** may already contain file descriptors. The given **a_sock** is added
 * to the set. It is ensured that \p \::select() is not fed with invalid socket
 * file descriptors, in particular with the EBADF error. That could mean: closed
 * socket, or an other network error was detected before adding to the set. It
 * checks that we do not use closed or unbind sockets. It also has a guard that
 * we do not exceed the maximum number FD_SETSIZE (1024) of selectable file
 * descriptors, as noted in "man select".
 *
 * **Returns**
 *  - Nothing. Not good, but needed for compatibility.\n
 * You can check if **a_sock** was added to **a_set**. There are messages to
 * stderr if verbose logging is enabled.
 */
void fdset_if_valid( //
    SOCKET a_sock,   ///< [in] socket file descriptor.
    fd_set* a_set /*!< [out] Pointer to an \p 'fd_set' structure as needed for
                     \p \::select(). The structure is modified as documented for
                     \p \::select(). */
) {
    UPNPLIB_LOGINFO "MSG1086: Check sockfd=" << a_sock << ".\n";
    if (a_sock == INVALID_SOCKET)
        // This is a defined state and we return silently.
        return;

    if (a_sock < 3 || a_sock >= FD_SETSIZE) {
        UPNPLIB_LOGERR "MSG1005: "
            << (a_sock < 0 ? "Invalid" : "Prohibited") << " socket " << a_sock
            << " not set to be monitored by ::select()"
            << (a_sock >= 3 ? " because it violates FD_SETSIZE.\n" : ".\n");
        return;
    }
    // Check if socket is valid and bound
    try {
        upnplib::CSocket_basic sockObj(a_sock);
        sockObj.init();
        if (sockObj.is_bound())

            FD_SET(a_sock, a_set);

        else
            UPNPLIB_LOGINFO "MSG1002: Unbound socket "
                << a_sock << " not set to be monitored by ::select().\n";

    } catch (const std::exception& e) {
        if (upnplib::g_dbug)
            std::clog << e.what();
        UPNPLIB_LOGCATCH "MSG1009: Invalid socket "
            << a_sock << " not set to be monitored by ::select().\n";
    }
}

/*!
 * \brief Accept requested connection from a remote control point and run it in
 * a new thread.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_NO_WEB_SERVER
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_ACCEPT
 */
int web_server_accept(
    /// [in] Socket file descriptor.
    [[maybe_unused]] SOCKET lsock,
    /// [out] Reference to a file descriptor set as needed for \::select().
    [[maybe_unused]] fd_set& set) {
#ifndef COMPA_HAVE_WEBSERVER
    return UPNP_E_NO_WEB_SERVER;
#else
    TRACE("Executing web_server_accept()")
    if (lsock == INVALID_SOCKET || !FD_ISSET(lsock, &set)) {
        UPNPLIB_LOGINFO "MSG1012: Socket("
            << lsock << ") invalid or not in file descriptor set.\n";
        return UPNP_E_SOCKET_ERROR;
    }

    SOCKET asock;
    upnplib::sockaddr_t clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // accept a network request connection
    asock = umock::sys_socket_h.accept(lsock, &clientAddr.sa, &clientLen);
    if (asock == INVALID_SOCKET) {
        UPNPLIB_LOGERR "MSG1022: Error in ::accept(): " << std::strerror(errno)
                                                        << ".\n";
        return UPNP_E_SOCKET_ACCEPT;
    }

    // Schedule a job to manage a UPnP request from a remote host.
    char buf_ntop[INET6_ADDRSTRLEN + 7];
    inet_ntop(AF_INET, &clientAddr.sin.sin_addr, buf_ntop, sizeof(buf_ntop));
    UPNPLIB_LOGINFO "MSG1023: Connected to host "
        << buf_ntop << ":" << ntohs(clientAddr.sin.sin_port) << " with socket "
        << asock << ".\n";
    schedule_request_job(asock, &clientAddr.sa);

    return UPNP_E_SUCCESS;
#endif /* COMPA_HAVE_WEBSERVER */
}

/*!
 * \brief Read data from the SSDP socket.
 */
void ssdp_read( //
    SOCKET* rsock,     ///< [in] Pointer to a Socket file descriptor.
    fd_set* set        /*!< [in] Pointer to a file descriptor set as needed for
                                 \::select(). */) {
    TRACE("Executing ssdp_read()")
    if (*rsock == INVALID_SOCKET || !FD_ISSET(*rsock, set))
        return;

#if defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP)
    if (readFromSSDPSocket(*rsock) != 0) {
        UpnpPrintf(UPNP_ERROR, MSERV, __FILE__, __LINE__,
                   "miniserver: Error in readFromSSDPSocket(%d): "
                   "closing socket\n",
                   *rsock);
        sock_close(*rsock);
        *rsock = INVALID_SOCKET;
    }
#else
    sock_close(*rsock);
    *rsock = INVALID_SOCKET;
#endif
}

/*!
 * \brief Check if we have received a packet that shall stop the miniserver.
 *
 * The received datagram must exactly match the shutdown_str from 127.0.0.1.
 * This is a security issue to avoid that the UPnPlib can be terminated from a
 * remote ip address. Receiving 0 bytes on a datagram (there's a datagram here)
 * indicates that a zero-length datagram was successful sent. This will not
 * stop the miniserver.
 *
 * \returns
 * - 1 - when the miniserver shall be stopped,
 * - 0 - otherwise.
 */
int receive_from_stopSock(
    SOCKET ssock, ///< [in] Socket file descriptor.
    fd_set* set   /*!< [in] Pointer to a file descriptor set as needed for
                            \::select(). */
) {
    TRACE("Executing receive_from_stopSock()")
    constexpr char shutdown_str[]{"ShutDown"};

    if (!FD_ISSET(ssock, set))
        return 0; // Nothing to do for this socket

    upnplib::sockaddr_t clientAddr{};
    socklen_t clientLen{sizeof(clientAddr)}; // May be modified

    // The receive buffer is one byte greater with '\0' than the max receiving
    // bytes so the received message will always be terminated.
    char receiveBuf[sizeof(shutdown_str) + 1]{};
    char buf_ntop[INET6_ADDRSTRLEN];

    // receive from
    SSIZEP_T byteReceived = umock::sys_socket_h.recvfrom(
        ssock, receiveBuf, sizeof(shutdown_str), 0, &clientAddr.sa, &clientLen);
    if (byteReceived == SOCKET_ERROR ||
        inet_ntop(AF_INET, &clientAddr.sin.sin_addr, buf_ntop,
                  sizeof(buf_ntop)) == nullptr) {
        UPNPLIB_LOGCRIT "MSG1038: Failed to receive data from socket "
            << ssock << ". Stop miniserver.\n";
        return 1;
    }

    // 16777343 are netorder bytes of "127.0.0.1"
    if (clientAddr.sin.sin_addr.s_addr != 16777343 ||
        strcmp(receiveBuf, shutdown_str) != 0) //
    {
        char nullstr[]{"\\0"};
        if (byteReceived == 0 || receiveBuf[byteReceived - 1] != '\0')
            nullstr[0] = '\0';
        UPNPLIB_LOGERR "MSG1039: Received \""
            << receiveBuf << nullstr << "\" from " << buf_ntop << ":"
            << ntohs(clientAddr.sin.sin_port)
            << ", must be \"ShutDown\\0\" from 127.0.0.1:*. Don't "
               "stopping miniserver.\n";
        return 0;
    }

    UPNPLIB_LOGINFO "MSG1040: On socket "
        << ssock << " received ordinary datagram \"" << receiveBuf
        << "\\0\" from " << buf_ntop << ":" << ntohs(clientAddr.sin.sin_port)
        << ". Stop miniserver.\n";
    return 1;
}

/*!
 * \brief Run the miniserver.
 *
 * The MiniServer accepts a new request and schedules a thread to handle the
 * new request. It checks for socket state and invokes appropriate read and
 * shutdown actions for the Miniserver and SSDP sockets. This function itself
 * runs in its own thread.
 *
 * \attention The miniSock parameter must be allocated on the heap before
 * calling the function because it is freed by it.
 */
void RunMiniServer(
    /*! [in] Pointer to an Array containing valid sockets associated with
       different tasks like listen on a local interface for requests from
       control points or handle ssdp communication to a remote UPnP node. */
    MiniServerSockArray* miniSock) {
    UPNPLIB_LOGINFO "MSG1085: Executing...\n";
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
#ifdef COMPA_HAVE_CTRLPT_SSDP
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpReqSock4 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpReqSock4);
    maxMiniSock = //
        std::max(maxMiniSock, miniSock->ssdpReqSock6 == INVALID_SOCKET
                                  ? 0
                                  : miniSock->ssdpReqSock6);
#endif
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
#ifdef COMPA_HAVE_CTRLPT_SSDP
        fdset_if_valid(miniSock->ssdpReqSock4, &rdSet);
        fdset_if_valid(miniSock->ssdpReqSock6, &rdSet);
#endif

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
                // REF:_[Should_I_assert_fail_on_select()_EBADF?](https://stackoverflow.com/q/28015859/5014688)
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

        // Accept requested connection from a remote control point and run the
        // connection in a new thread. Due to side effects with threading we
        // need to avoid lazy evaluation with chained || because all
        // web_server_accept() must be called.
        // if (ret1 == UPNP_E_SUCCESS || ret2 == UPNP_E_SUCCESS ||
        //     ret3 == UPNP_E_SUCCESS) {
        [[maybe_unused]] int ret1 =
            web_server_accept(miniSock->miniServerSock4, rdSet);
        [[maybe_unused]] int ret2 =
            web_server_accept(miniSock->miniServerSock6, rdSet);
        [[maybe_unused]] int ret3 =
            web_server_accept(miniSock->miniServerSock6UlaGua, rdSet);
#ifdef COMPA_HAVE_CTRLPT_SSDP
        ssdp_read(&miniSock->ssdpReqSock4, &rdSet);
        ssdp_read(&miniSock->ssdpReqSock6, &rdSet);
#endif
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
#ifdef COMPA_HAVE_CTRLPT_SSDP
    sock_close(miniSock->ssdpReqSock4);
    sock_close(miniSock->ssdpReqSock6);
#endif
    /* Free minisock. */
    umock::stdlib_h.free(miniSock);
    gMServState = MSERV_IDLE;

    return;
}

void RunMiniServer_f(MiniServerSockArray* miniSock) {
    umock::pupnp_miniserver.RunMiniServer(miniSock);
}


/*!
 * \brief Returns port to which socket, sockfd, is bound.
 *
 * \returns
 *  On success: **0**
 *  On error: **-1** with unmodified system error (errno or WSAGetLastError()).
 */
int get_port(
    /*! [in] Socket descriptor. */
    SOCKET sockfd,
    /*! [out] The port value if successful, otherwise, untouched. */
    uint16_t* port) {
    TRACE("Executing get_port(), calls system ::getsockname()")
    upnplib::sockaddr_t sockinfo{};
    socklen_t len(sizeof sockinfo); // May be modified by getsockname()

    if (umock::sys_socket_h.getsockname(sockfd, &sockinfo.sa, &len) == -1)
        // system error (errno etc.) is expected to be unmodified on return.
        return -1;

    switch (sockinfo.ss.ss_family) {
    case AF_INET:
        *port = ntohs(sockinfo.sin.sin_port);
        break;
    case AF_INET6:
        *port = ntohs(sockinfo.sin6.sin6_port);
        break;
    default:
        // system error (errno etc.) is expected to be unmodified on return.
        return -1;
    }
    UPNPLIB_LOGINFO "MSG1063: sockfd=" << sockfd << ", port=" << *port << ".\n";

    return 0;
}

#ifdef COMPA_HAVE_WEBSERVER
/*!
 * \brief Create STREAM sockets, binds to local network interfaces and listens
 * for incoming connections.
 *
 * Only available with the webserver compiled in.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET - Failed to create a socket.
 *  - UPNP_E_SOCKET_BIND - Bind() failed.
 *  - UPNP_E_LISTEN - Listen() failed.
 *  - UPNP_E_INTERNAL_ERROR - Port returned by the socket layer is < 0.
 */
int get_miniserver_sockets(
    /*! [in,out] [in]Pointer to a Socket Array that following members will be
     * filled:
     *      - .miniServerSock6
     *      - .miniServerPort6
     *      - .miniServerSock6UlaGua
     *      - .miniServerPort6UlaGua
     *      - .miniServerSock4
     *      - .miniServerPort4 */
    MiniServerSockArray* out,
    /*! [in] port on which the UPnP Device shall listen for incoming IPv4
       connections. If **0** then a random port number is returned in **out**.
     */
    in_port_t listen_port4,
    /*! [in] port on which the UPnP Device shall listen for incoming IPv6
       [LLA](\ref glossary_ipv6addr) connections. If **0** then a random port
       number is returned in **out**. */
    in_port_t listen_port6,
    /*! [in] port on which the UPnP Device shall listen for incoming IPv6
       [UAD](\ref glossary_ipv6addr) connections. If **0** then a random port
       number is returned in ***out**. */
    in_port_t listen_port6UlaGua) {
    UPNPLIB_LOGINFO "MSG1109: Executing with listen_port4="
        << listen_port4 << ", listen_port6=" << listen_port6
        << ", listen_port6UlaGua=" << listen_port6UlaGua << ".\n";

    int retval{UPNP_E_OUTOF_SOCKET};

    if (out->MiniSvrSock6LlaObj != nullptr && gIF_IPV6[0] != '\0') {
        try {
            out->MiniSvrSock6LlaObj->init();
            out->MiniSvrSock6LlaObj->bind('[' + std::string(gIF_IPV6) + ']',
                                          std::to_string(listen_port6));
            out->MiniSvrSock6LlaObj->listen();
            out->miniServerSock6 = *out->MiniSvrSock6LlaObj;
            out->miniServerPort6 = out->MiniSvrSock6LlaObj->get_port();
            retval = UPNP_E_SUCCESS;
        } catch (const std::exception& e) {
            UPNPLIB_LOGCATCH "MSG1110: catched next line...\n" << e.what();
        }
    }

    // ss6.serverAddr6->sin6_scope_id = gIF_INDEX;
    // I could not find where sin6_scope_id is read elsewhere to use it.
    // It is never copied to 'out'. --Ingo

    if (out->MiniSvrSock6UadObj != nullptr && gIF_IPV6_ULA_GUA[0] != '\0') {
        try {
            out->MiniSvrSock6UadObj->init();
            out->MiniSvrSock6UadObj->bind('[' + std::string(gIF_IPV6_ULA_GUA) +
                                              ']',
                                          std::to_string(listen_port6UlaGua));
            out->MiniSvrSock6UadObj->listen();
            out->miniServerSock6UlaGua = *out->MiniSvrSock6UadObj;
            out->miniServerPort6UlaGua = out->MiniSvrSock6UadObj->get_port();
            retval = UPNP_E_SUCCESS;
        } catch (const std::exception& e) {
            UPNPLIB_LOGCATCH "MSG1117: catched next line...\n" << e.what();
        }
    }

    if (out->MiniSvrSock4Obj != nullptr && gIF_IPV4[0] != '\0') {
        try {
            out->MiniSvrSock4Obj->init();
            out->MiniSvrSock4Obj->bind(std::string(gIF_IPV4),
                                       std::to_string(listen_port4));
            out->MiniSvrSock4Obj->listen();
            out->miniServerSock4 = *out->MiniSvrSock4Obj;
            out->miniServerPort4 = out->MiniSvrSock4Obj->get_port();
            retval = UPNP_E_SUCCESS;
        } catch (const std::exception& e) {
            UPNPLIB_LOGCATCH "MSG1112: catched next line...\n" << e.what();
        }
    }

    UPNPLIB_LOGINFO "MSG1065: Finished.\n";
    return retval;
}
#endif /* COMPA_HAVE_WEBSERVER */

/*!
 * \brief Creates the miniserver STOP socket, usable to listen for stopping the
 * miniserver.
 *
 * This datagram socket is created and bound to "localhost". It does not listen
 * now but will later be used to listen on to know when it is time to stop the
 * Miniserver.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET: Failed to create a socket.
 *  - UPNP_E_SOCKET_BIND: Bind() failed.
 *  - UPNP_E_INTERNAL_ERROR: Port returned by the socket layer is < 0.
 */
int get_miniserver_stopsock(
    /*! [out] Fills the socket file descriptor and the port it is bound to into
       the structure. */
    MiniServerSockArray* out) {
    TRACE("Executing get_miniserver_stopsock()");
    sockaddr_in stop_sockaddr;

    upnplib::CSocketErr sockerrObj;
    SOCKET miniServerStopSock =
        umock::sys_socket_h.socket(AF_INET, SOCK_DGRAM, 0);
    if (miniServerStopSock == INVALID_SOCKET) {
        sockerrObj.catch_error();
        UPNPLIB_LOGCRIT "MSG1094: Error in socket(): " << sockerrObj.error_str()
                                                       << "\n";
        return UPNP_E_OUTOF_SOCKET;
    }
    /* Bind to local socket. */
    memset(&stop_sockaddr, 0, sizeof(stop_sockaddr));
    stop_sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &stop_sockaddr.sin_addr);
    int ret = umock::sys_socket_h.bind(
        miniServerStopSock, reinterpret_cast<sockaddr*>(&stop_sockaddr),
        sizeof(stop_sockaddr));
    if (ret == SOCKET_ERROR) {
        sockerrObj.catch_error();
        UPNPLIB_LOGCRIT "MSG1095: Error in binding localhost: "
            << sockerrObj.error_str() << "\n";
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

    UPNPLIB_LOGINFO "MSG1053: Bound stop socket="
        << miniServerStopSock << " to \"127.0.0.1:" << miniStopSockPort
        << "\".\n";

    return UPNP_E_SUCCESS;
}

/*!
 * \brief Initialize a miniserver Socket Array.
 */
void InitMiniServerSockArray(
    MiniServerSockArray* miniSocket ///< Pointer to a miniserver Socket Array.
) {
    TRACE("Executing InitMiniServerSockArray()");
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
#ifdef COMPA_HAVE_CTRLPT_SSDP
    miniSocket->ssdpReqSock4 = INVALID_SOCKET;
    miniSocket->ssdpReqSock6 = INVALID_SOCKET;
#endif
}

/// @} // Functions (scope restricted to file)
} // anonymous namespace


#ifdef COMPA_HAVE_DEVICE_SOAP
void SetSoapCallback(MiniServerCallback callback) {
    TRACE("Executing SetSoapCallback()");
    gSoapCallback = callback;
}
#endif

#ifdef COMPA_HAVE_DEVICE_GENA
void SetGenaCallback(MiniServerCallback callback) {
    TRACE("Executing SetGenaCallback()");
    gGenaCallback = callback;
}
#endif

int StartMiniServer([[maybe_unused]] in_port_t* listen_port4,
                    [[maybe_unused]] in_port_t* listen_port6,
                    [[maybe_unused]] in_port_t* listen_port6UlaGua) {
    UPNPLIB_LOGINFO "MSG1068: Executing...\n";
    constexpr int max_count{10000};
    MiniServerSockArray* miniSocket;
    ThreadPoolJob job;
    int ret_code{UPNP_E_INTERNAL_ERROR};

    memset(&job, 0, sizeof(job));

    if (gMServState != MSERV_IDLE) {
        /* miniserver running. */
        UPNPLIB_LOGERR "MSG1087: Cannot start. Miniserver is running.\n";
        return UPNP_E_INTERNAL_ERROR;
    }
    miniSocket = (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
    if (!miniSocket) {
        return UPNP_E_OUTOF_MEMORY;
    }
    InitMiniServerSockArray(miniSocket);

    // Instantiate socket objects and point to them in miniSocket
    upnplib::CSocket Sock6LlaObj(AF_INET6, SOCK_STREAM);
    miniSocket->MiniSvrSock6LlaObj = &Sock6LlaObj;
    upnplib::CSocket Sock6UadObj(AF_INET6, SOCK_STREAM);
    miniSocket->MiniSvrSock6UadObj = &Sock6UadObj;
    upnplib::CSocket Sock4Obj(AF_INET, SOCK_STREAM);
    miniSocket->MiniSvrSock4Obj = &Sock4Obj;

#ifdef COMPA_HAVE_WEBSERVER
    if (*listen_port4 == 0 || *listen_port6 == 0 || *listen_port6UlaGua == 0) {
        // Create a simple random number generator for port numbers. We need
        // this because we do not reuse addresses before TIME_WAIT has expired
        // (socket option SO_REUSEADDR = false). We need to use different
        // socket addresses and that is already given with different port
        // numbers.
        std::random_device rd;         // obtain a random number from hardware
        std::minstd_rand random(rd()); // seed the generator
        std::uniform_int_distribution<in_port_t> portno(
            APPLICATION_LISTENING_PORT, 65535); // used range

        in_port_t listen_port = portno(random);
        if (*listen_port4 == 0)
            *listen_port4 = listen_port;
        if (*listen_port6 == 0)
            *listen_port6 = listen_port;
        if (*listen_port6UlaGua == 0)
            *listen_port6UlaGua = listen_port;
    }
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
#if defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP)
    /* SSDP socket for discovery/advertising. */
    ret_code = umock::pupnp_ssdp.get_ssdp_sockets(miniSocket);
    if (ret_code != UPNP_E_SUCCESS) {
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        sock_close(miniSocket->miniServerStopSock);
        free(miniSocket);
        return ret_code;
    }
#endif
    TPJobInit(&job, (start_routine)RunMiniServer_f, (void*)miniSocket);
    TPJobSetPriority(&job, MED_PRIORITY);
    TPJobSetFreeFunction(&job, (free_routine)free);
    ret_code = ThreadPoolAddPersistent(&gMiniServerThreadPool, &job, NULL);
    if (ret_code != 0) {
        sock_close(miniSocket->miniServerSock4);
        sock_close(miniSocket->miniServerSock6);
        sock_close(miniSocket->miniServerSock6UlaGua);
        sock_close(miniSocket->miniServerStopSock);
        sock_close(miniSocket->ssdpSock4);
        sock_close(miniSocket->ssdpSock6);
        sock_close(miniSocket->ssdpSock6UlaGua);
#ifdef COMPA_HAVE_CTRLPT_SSDP
        sock_close(miniSocket->ssdpReqSock4);
        sock_close(miniSocket->ssdpReqSock6);
#endif
        free(miniSocket);
        return UPNP_E_OUTOF_MEMORY;
    }
    /* Wait for miniserver to start. */
    int count{0};
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
#ifdef COMPA_HAVE_CTRLPT_SSDP
        sock_close(miniSocket->ssdpReqSock4);
        sock_close(miniSocket->ssdpReqSock6);
#endif
        return UPNP_E_INTERNAL_ERROR;
    }
#ifdef COMPA_HAVE_WEBSERVER
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
        return 1;
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
