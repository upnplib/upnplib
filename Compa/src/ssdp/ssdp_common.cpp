/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2011-2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-05
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
// Last compare with ./pupnp source file on 2023-08-30, ver 1.14.18
/*!
 * \file
 * \brief Manage "Step 1: Discovery" of the UPnP+™ specification for common SSDP
 * usage.
 *
 * \ingroup compa-Discovery
 */

#include <ssdp_ctrlpt.hpp>
#include <ssdp_device.hpp>
#include <upnpapi.hpp>

#ifndef COMPA_SSDP_COMMON_HPP
#error "No or wrong ssdp_common.hpp header file included."
#endif
#ifndef COMPA_INTERNAL_CONFIG_HPP
#error "No or wrong config.hpp header file included."
#endif

#include <upnplib/global.hpp> // for TRACE
#include <umock/sys_socket.hpp>
#include <umock/winsock2.hpp>

/// \cond
#if UPNPLIB_WITH_TRACE
#include <iostream>
#endif
/// \endcond

namespace {
/*! \name Functions scope restricted to file
 * @{ */

/*!
 * \brief Frees the ssdp request.
 */
void free_ssdp_event_handler_data(
    /*! [in] ssdp_thread_data structure. This structure contains SSDP request
       message. */
    void* the_data) {
    ssdp_thread_data* data = (ssdp_thread_data*)the_data;

    if (data != NULL) {
        http_message_t* hmsg = &data->parser.msg;
        /* free data */
        httpmsg_destroy(hmsg);
        free(data);
    }
}

/*!
 * \brief Does some quick checking of the ssdp msg.
 *
 * \returns **1** if msg is valid, else **0**.
 */
inline int valid_ssdp_msg(
    /*! [in] ssdp_thread_data structure. This structure contains SSDP request
       message. */
    http_message_t* hmsg) {
    memptr hdr_value;

    /* check for valid methods - NOTIFY or M-SEARCH */
    if (hmsg->method != (http_method_t)HTTPMETHOD_NOTIFY &&
        hmsg->method != (http_method_t)HTTPMETHOD_MSEARCH &&
        hmsg->request_method != (http_method_t)HTTPMETHOD_MSEARCH) {
        return 0;
    }
    if (hmsg->request_method != (http_method_t)HTTPMETHOD_MSEARCH) {
        /* check PATH == "*" */
        if (hmsg->uri.type != (enum uriType)RELATIVE ||
            strncmp("*", hmsg->uri.pathquery.buff, hmsg->uri.pathquery.size) !=
                0) {
            return 0;
        }
        /* check HOST header */
        if (httpmsg_find_hdr(hmsg, HDR_HOST, &hdr_value) == NULL ||
            (memptr_cmp(&hdr_value, "239.255.255.250:1900") != 0 &&
             memptr_cmp(&hdr_value, "[FF02::C]:1900") != 0 &&
             memptr_cmp(&hdr_value, "[ff02::c]:1900") != 0 &&
             memptr_cmp(&hdr_value, "[FF05::C]:1900") != 0 &&
             memptr_cmp(&hdr_value, "[ff05::c]:1900") != 0)) {
            UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
                       "Invalid HOST header from SSDP message\n");

            return 0;
        }
    }

    /* passed quick check */
    return 1;
}

/*!
 * \brief Parses the message and dispatches it to a handler which handles the
 * ssdp request msg.
 *
 * \returns
 *  On success: **0**\n
 *  On error: **-1**
 */
inline int start_event_handler(
    /*! [in] ssdp_thread_data structure. This structure contains SSDP request
       message. */
    void* Data) {
    http_parser_t* parser = NULL;
    parse_status_t status;
    ssdp_thread_data* data = (ssdp_thread_data*)Data;

    parser = &data->parser;
    status = parser_parse(parser);
    if (status == (parse_status_t)PARSE_FAILURE) {
        if (parser->msg.method != (http_method_t)HTTPMETHOD_NOTIFY ||
            !parser->valid_ssdp_notify_hack) {
            UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
                       "SSDP recvd bad msg code = %d\n", status);
            /* ignore bad msg, or not enuf mem */
            goto error_handler;
        }
        /* valid notify msg */
    } else if (status != (parse_status_t)PARSE_SUCCESS) {
        UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
                   "SSDP recvd bad msg code = %d\n", status);

        goto error_handler;
    }
    /* check msg */
    if (valid_ssdp_msg(&parser->msg) != 1) {
        goto error_handler;
    }
    /* done; thread will free 'data' */
    return 0;

error_handler:
    free_ssdp_event_handler_data(data);
    return -1;
}

/*!
 * \brief This function is a thread that handles SSDP requests.
 */
void ssdp_event_handler_thread(
    /*! [] Ssdp_thread_data structure. This structure contains SSDP request
       message. */
    void* the_data) {
    ssdp_thread_data* data = (ssdp_thread_data*)the_data;
    http_message_t* hmsg = &data->parser.msg;

    if (start_event_handler(the_data) != 0)
        return;
    /* send msg to device or ctrlpt */
    if (hmsg->method == (http_method_t)HTTPMETHOD_NOTIFY ||
        hmsg->request_method == (http_method_t)HTTPMETHOD_MSEARCH) {
/// \todo Make utests and split function to separate handle ctrlpt and service.
#ifdef COMPA_HAVE_CTRLPT_SSDP
        ssdp_handle_ctrlpt_msg(hmsg, &data->dest_addr, 0);
#endif
    } else {
#ifdef COMPA_HAVE_DEVICE_SSDP
        ssdp_handle_device_request(hmsg, &data->dest_addr);
#endif
    }

    /* free data */
    free_ssdp_event_handler_data(data);
}

/*!
 * \brief Create an SSDP IPv4 socket.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_BIND
 *  - UPNP_E_NETWORK_ERROR
 */
inline int create_ssdp_sock_v4(
    /*! [out] SSDP IPv4 socket to be created. */
    SOCKET* ssdpSock) {
    TRACE("Executing create_ssdp_sock_v4()")
    char errorBuffer[ERROR_BUFFER_LEN];
    int onOff;
    u_char ttl = (u_char)4;
    struct ip_mreq ssdpMcastAddr;
    struct sockaddr_storage __ss;
    struct sockaddr_in* ssdpAddr4 = (struct sockaddr_in*)&__ss;
    int ret = 0;
    struct in_addr addr;

    *ssdpSock = umock::sys_socket_h.socket(AF_INET, SOCK_DGRAM, 0);
    if (*ssdpSock == INVALID_SOCKET) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in socket(): %s\n", errorBuffer);

        return UPNP_E_OUTOF_SOCKET;
    }
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEADDR,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEADDR: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#if (defined(BSD) && !defined(__GNU__)) || defined(__APPLE__)
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEPORT,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEPORT: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#endif /* BSD, __APPLE__ */
    memset(&__ss, 0, sizeof(__ss));
    ssdpAddr4->sin_family = (sa_family_t)AF_INET;
    ssdpAddr4->sin_addr.s_addr = htonl(INADDR_ANY);
    ssdpAddr4->sin_port = htons(SSDP_PORT);
    ret = umock::sys_socket_h.bind(*ssdpSock, (sockaddr*)ssdpAddr4,
                                   sizeof(*ssdpAddr4));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in bind(), addr=0x%08X, port=%d: %s\n", INADDR_ANY,
                   SSDP_PORT, errorBuffer);
        ret = UPNP_E_SOCKET_BIND;
        goto error_handler;
    }
    /*
     * See: https://man7.org/linux/man-pages/man7/ip.7.html
     * Socket options, IP_ADD_MEMBERSHIP
     *
     * This memset actually sets imr_address to INADDR_ANY and
     * imr_ifindex to zero, which make the system choose the appropriate
     * interface.
     *
     * Still using "struct ip_mreq" instead of "struct ip_mreqn" because
     * windows does not recognize the latter.
     */
    memset((void*)&ssdpMcastAddr, 0, sizeof ssdpMcastAddr);
#ifdef _WIN32
    inet_pton(AF_INET, (PCSTR)gIF_IPV4, &ssdpMcastAddr.imr_interface);
    inet_pton(AF_INET, (PCSTR)SSDP_IP, &ssdpMcastAddr.imr_multiaddr);
#else
    ssdpMcastAddr.imr_interface.s_addr = inet_addr(gIF_IPV4);
    /* ssdpMcastAddr.imr_address.s_addr = inet_addr(gIF_IPV4); */
    ssdpMcastAddr.imr_multiaddr.s_addr = inet_addr(SSDP_IP);
#endif
    ret = umock::sys_socket_h.setsockopt(
        *ssdpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ssdpMcastAddr,
        sizeof(struct ip_mreq));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IP_ADD_MEMBERSHIP (join "
                   "multicast group): %s\n",
                   errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
    /* Set multicast interface. */
    memset((void*)&addr, 0, sizeof(struct in_addr));
#ifdef _WIN32
    inet_pton(AF_INET, (PCSTR)gIF_IPV4, &addr);
#else
    addr.s_addr = inet_addr(gIF_IPV4);
#endif
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IP, IP_MULTICAST_IF,
                                         (char*)&addr, sizeof addr);
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IP_MULTICAST_IF (set multicast "
                   "interface): %s\n",
                   errorBuffer);
        /* This is probably not a critical error, so let's continue. */
    }
    /* result is not checked becuase it will fail in WinMe and Win9x. */
    umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IP, IP_MULTICAST_TTL,
                                   (const char*)&ttl, sizeof(ttl));
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_BROADCAST,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_BROADCAST (set broadcast): "
                   "%s\n",
                   errorBuffer);
        ret = UPNP_E_NETWORK_ERROR;
        goto error_handler;
    }
    ret = UPNP_E_SUCCESS;

error_handler:
    if (ret != UPNP_E_SUCCESS) {
        UpnpCloseSocket(*ssdpSock);
    }

    return ret;
}

#ifdef UPNP_ENABLE_IPV6
/*!
 * \brief Create an SSDP IPv6 socket.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_BIND
 *  - UPNP_E_NETWORK_ERROR
 */
inline int create_ssdp_sock_v6(
    /*! [out] SSDP IPv6 socket to be created. */
    SOCKET* ssdpSock) {
    char errorBuffer[ERROR_BUFFER_LEN];
    struct ipv6_mreq ssdpMcastAddr;
    struct sockaddr_storage __ss;
    struct sockaddr_in6* ssdpAddr6 = (struct sockaddr_in6*)&__ss;
    int onOff;
    int ret = 0;

    *ssdpSock = umock::sys_socket_h.socket(AF_INET6, SOCK_DGRAM, 0);
    if (*ssdpSock == INVALID_SOCKET) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in socket(): %s\n", errorBuffer);

        return UPNP_E_OUTOF_SOCKET;
    }
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEADDR,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEADDR: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#if (defined(BSD) && !defined(__GNU__)) || defined(__APPLE__)
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEPORT,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEPORT: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#endif /* BSD, __APPLE__ */
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IPV6, IPV6_V6ONLY,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IPV6_V6ONLY: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
    memset(&__ss, 0, sizeof(__ss));
    ssdpAddr6->sin6_family = (sa_family_t)AF_INET6;
    ssdpAddr6->sin6_addr = in6addr_any;
#ifndef _WIN32
    ssdpAddr6->sin6_scope_id = gIF_INDEX;
#endif
    ssdpAddr6->sin6_port = htons(SSDP_PORT);
    ret = umock::sys_socket_h.bind(*ssdpSock, (struct sockaddr*)ssdpAddr6,
                                   sizeof(*ssdpAddr6));
    if (ret == -1) {
#ifndef _WIN32
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in bind(), addr=%s, index=%d, port=%d: %s\n",
                   gIF_IPV6, gIF_INDEX, SSDP_PORT, errorBuffer);
        ret = UPNP_E_SOCKET_BIND;
        goto error_handler;
#else
        int wsa_err = umock::winsock2_h.WSAGetLastError();
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in bind(), addr=%s, index=%d, port=%d: %d\n",
                   gIF_IPV6, gIF_INDEX, SSDP_PORT, wsa_err);
        ret = UPNP_E_SOCKET_BIND;
        goto error_handler;
#endif
    }
    memset((void*)&ssdpMcastAddr, 0, sizeof(ssdpMcastAddr));
    ssdpMcastAddr.ipv6mr_interface = gIF_INDEX;
    inet_pton(AF_INET6, SSDP_IPV6_LINKLOCAL, &ssdpMcastAddr.ipv6mr_multiaddr);
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IPV6,
                                         IPV6_JOIN_GROUP, (char*)&ssdpMcastAddr,
                                         sizeof(ssdpMcastAddr));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        uint32_t* p = (uint32_t*)&ssdpMcastAddr.ipv6mr_multiaddr;
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IPV6_JOIN_GROUP (join multicast "
                   "group): %s.\n"
                   "SSDP_IPV6_LINKLOCAL = %s,\n"
                   "ipv6mr_interface = %u,\n"
                   "ipv6mr_multiaddr[0,1,2,3] = "
                   "0x%08X:0x%08X:0x%08X:0x%08X\n"
                   "gIF_NAME = %s\n",
                   errorBuffer, SSDP_IPV6_LINKLOCAL,
                   ssdpMcastAddr.ipv6mr_interface, p[0], p[1], p[2], p[3],
                   gIF_NAME);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_BROADCAST,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_BROADCAST (set broadcast): "
                   "%s\n",
                   errorBuffer);
        ret = UPNP_E_NETWORK_ERROR;
        goto error_handler;
    }
    ret = UPNP_E_SUCCESS;

error_handler:
    if (ret != UPNP_E_SUCCESS) {
        UpnpCloseSocket(*ssdpSock);
    }

    return ret;
}
#endif /* IPv6 */

#ifdef UPNP_ENABLE_IPV6
/*!
 * \brief Create an SSDP IPv6 ULA_GUA socket.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_BIND
 *  - UPNP_E_NETWORK_ERROR
 */
inline int create_ssdp_sock_v6_ula_gua(
    /*! [out] SSDP IPv6 socket to be created. */
    SOCKET* ssdpSock) {
    char errorBuffer[ERROR_BUFFER_LEN];
    struct ipv6_mreq ssdpMcastAddr;
    struct sockaddr_storage __ss;
    struct sockaddr_in6* ssdpAddr6 = (struct sockaddr_in6*)&__ss;
    int onOff;
    int ret = 0;

    *ssdpSock = umock::sys_socket_h.socket(AF_INET6, SOCK_DGRAM, 0);
    if (*ssdpSock == INVALID_SOCKET) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in socket(): %s\n", errorBuffer);

        return UPNP_E_OUTOF_SOCKET;
    }
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEADDR,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEADDR: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#if (defined(BSD) && !defined(__GNU__)) || defined(__APPLE__)
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEPORT,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_REUSEPORT: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
#endif /* BSD, __APPLE__ */
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IPV6, IPV6_V6ONLY,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IPV6_V6ONLY: %s\n", errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
    memset(&__ss, 0, sizeof(__ss));
    ssdpAddr6->sin6_family = (sa_family_t)AF_INET6;
    ssdpAddr6->sin6_addr = in6addr_any;
    ssdpAddr6->sin6_scope_id = gIF_INDEX;
    ssdpAddr6->sin6_port = htons(SSDP_PORT);
    ret = umock::sys_socket_h.bind(*ssdpSock, (struct sockaddr*)ssdpAddr6,
                                   sizeof(*ssdpAddr6));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in bind(), addr=0x%032lX, port=%d: %s\n", 0lu,
                   SSDP_PORT, errorBuffer);
        ret = UPNP_E_SOCKET_BIND;
        goto error_handler;
    }
    memset((void*)&ssdpMcastAddr, 0, sizeof(ssdpMcastAddr));
    ssdpMcastAddr.ipv6mr_interface = gIF_INDEX;
    /* SITE LOCAL */
    inet_pton(AF_INET6, SSDP_IPV6_SITELOCAL, &ssdpMcastAddr.ipv6mr_multiaddr);
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, IPPROTO_IPV6,
                                         IPV6_JOIN_GROUP, (char*)&ssdpMcastAddr,
                                         sizeof(ssdpMcastAddr));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() IPV6_JOIN_GROUP (join multicast "
                   "group): %s\n",
                   errorBuffer);
        ret = UPNP_E_SOCKET_ERROR;
        goto error_handler;
    }
    onOff = 1;
    ret = umock::sys_socket_h.setsockopt(*ssdpSock, SOL_SOCKET, SO_BROADCAST,
                                         (char*)&onOff, sizeof(onOff));
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in setsockopt() SO_BROADCAST (set broadcast): "
                   "%s\n",
                   errorBuffer);
        ret = UPNP_E_NETWORK_ERROR;
        goto error_handler;
    }
    ret = UPNP_E_SUCCESS;

error_handler:
    if (ret != UPNP_E_SUCCESS) {
        UpnpCloseSocket(*ssdpSock);
    }

    return ret;
}
#endif /* IPv6 */

/// @} // Functions scope restricted to file
} // anonymous namespace


int unique_service_name(char* cmd, SsdpEvent* Evt) {
    char TempBuf[LINE_SIZE - 3];
    char* TempPtr = NULL;
    char* Ptr = NULL;
    char* ptr1 = NULL;
    char* ptr2 = NULL;
    char* ptr3 = NULL;
    int CommandFound = 0;
    size_t n = (size_t)0;

    if (strstr(cmd, "uuid:schemas") != NULL) {
        ptr1 = strstr(cmd, ":device");
        if (ptr1 != NULL)
            ptr2 = strstr(ptr1 + 1, ":");
        else
            return -1;
        if (ptr2 != NULL)
            ptr3 = strstr(ptr2 + 1, ":");
        else
            return -1;
        if (ptr3 != NULL) {
            if (strlen("uuid:") + strlen(ptr3 + 1) >= sizeof Evt->UDN)
                return -1;
            snprintf(Evt->UDN, sizeof Evt->UDN, "uuid:%s", ptr3 + 1);
        } else
            return -1;
        ptr1 = strstr(cmd, ":");
        if (ptr1 != NULL) {
            n = (size_t)ptr3 - (size_t)ptr1;
            n = n >= sizeof TempBuf ? sizeof TempBuf - 1 : n;
            strncpy(TempBuf, ptr1, n);
            TempBuf[n] = '\0';
            if (strlen("urn") + strlen(TempBuf) >= sizeof(Evt->DeviceType))
                return -1;
            snprintf(Evt->DeviceType, sizeof(Evt->DeviceType), "urn%s",
                     TempBuf);
        } else
            return -1;
        return 0;
    }
    if ((TempPtr = strstr(cmd, "uuid")) != NULL) {
        if ((Ptr = strstr(cmd, "::")) != NULL) {
            n = (size_t)Ptr - (size_t)TempPtr;
            n = n >= sizeof Evt->UDN ? sizeof Evt->UDN - 1 : n;
            strncpy(Evt->UDN, TempPtr, n);
            Evt->UDN[n] = '\0';
        } else {
            memset(Evt->UDN, 0, sizeof(Evt->UDN));
            strncpy(Evt->UDN, TempPtr, sizeof Evt->UDN - 1);
        }
        CommandFound = 1;
    }
    if (strstr(cmd, "urn:") != NULL && strstr(cmd, ":service:") != NULL) {
        if ((TempPtr = strstr(cmd, "urn")) != NULL) {
            memset(Evt->ServiceType, 0, sizeof Evt->ServiceType);
            strncpy(Evt->ServiceType, TempPtr, sizeof Evt->ServiceType - 1);
            CommandFound = 1;
        }
    }
    if (strstr(cmd, "urn:") != NULL && strstr(cmd, ":device:") != NULL) {
        if ((TempPtr = strstr(cmd, "urn")) != NULL) {
            memset(Evt->DeviceType, 0, sizeof Evt->DeviceType);
            strncpy(Evt->DeviceType, TempPtr, sizeof Evt->DeviceType - 1);
            CommandFound = 1;
        }
    }
    if ((TempPtr = strstr(cmd, "::upnp:rootdevice")) != NULL) {
        /* Everything before "::upnp::rootdevice" is the UDN. */
        if (TempPtr != cmd) {
            n = (size_t)TempPtr - (size_t)cmd;
            n = n >= sizeof Evt->UDN ? sizeof Evt->UDN - 1 : n;
            strncpy(Evt->UDN, cmd, n);
            Evt->UDN[n] = 0;
            CommandFound = 1;
        }
    }
    if (CommandFound == 0)
        return -1;

    return 0;
}

SsdpSearchType ssdp_request_type1(char* cmd) {
    if (strstr(cmd, ":all"))
        return SSDP_ALL;
    if (strstr(cmd, ":rootdevice"))
        return SSDP_ROOTDEVICE;
    if (strstr(cmd, "uuid:"))
        return SSDP_DEVICEUDN;
    if (strstr(cmd, "urn:") && strstr(cmd, ":device:"))
        return SSDP_DEVICETYPE;
    if (strstr(cmd, "urn:") && strstr(cmd, ":service:"))
        return SSDP_SERVICE;
    return SSDP_SERROR;
}

int ssdp_request_type(char* cmd, SsdpEvent* Evt) {
    /* clear event */
    memset(Evt, 0, sizeof(SsdpEvent));
    unique_service_name(cmd, Evt);
    Evt->ErrCode = NO_ERROR_FOUND;
    if ((Evt->RequestType = ssdp_request_type1(cmd)) == SSDP_SERROR) {
        Evt->ErrCode = E_HTTP_SYNTEX;
        return -1;
    }
    return 0;
}


int readFromSSDPSocket(SOCKET socket) {
    TRACE("Executing readFromSSDPSocket()")
    char* requestBuf = NULL;
    char staticBuf[BUFSIZE];
    struct sockaddr_storage __ss;
    ThreadPoolJob job;
    ssdp_thread_data* data = NULL;
    socklen_t socklen = sizeof(__ss);
    ssize_t byteReceived = 0;
    char ntop_buf[INET6_ADDRSTRLEN];

    memset(&job, 0, sizeof(job));

    requestBuf = staticBuf;
    /* in case memory can't be allocated, still drain the socket using a
     * static buffer. */
    data = (ssdp_thread_data*)malloc(sizeof(ssdp_thread_data));
    if (data) {
        /* initialize parser */
#ifdef COMPA_HAVE_CTRLPT_SSDP
        if (socket == gSsdpReqSocket4
#ifdef UPNP_ENABLE_IPV6
            || socket == gSsdpReqSocket6
#endif /* UPNP_ENABLE_IPV6 */
        )
            parser_response_init(&data->parser, HTTPMETHOD_MSEARCH);
        else
            parser_request_init(&data->parser);
#else  /* COMPA_HAVE_CTRLPT_SSDP */
        parser_request_init(&data->parser);
#endif /* COMPA_HAVE_CTRLPT_SSDP */
        /* set size of parser buffer */
        if (membuffer_set_size(&data->parser.msg.msg, BUFSIZE) == 0)
            /* use this as the buffer for recv */
            requestBuf = data->parser.msg.msg.buf;
        else {
            free(data);
            data = NULL;
        }
    }
    byteReceived =
        umock::sys_socket_h.recvfrom(socket, requestBuf, BUFSIZE - (size_t)1, 0,
                                     (struct sockaddr*)&__ss, &socklen);
    if (byteReceived > 0) {
        requestBuf[byteReceived] = '\0';
        switch (__ss.ss_family) {
        case AF_INET:
            inet_ntop(AF_INET, &((struct sockaddr_in*)&__ss)->sin_addr,
                      ntop_buf, sizeof(ntop_buf));
            break;
#ifdef UPNP_ENABLE_IPV6
        case AF_INET6:
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)&__ss)->sin6_addr,
                      ntop_buf, sizeof(ntop_buf));
            break;
#endif /* UPNP_ENABLE_IPV6 */
        default:
            memset(ntop_buf, 0, sizeof(ntop_buf));
            strncpy(ntop_buf, "<Invalid address family>", sizeof(ntop_buf) - 1);
        }
        /* clang-format off */
        UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
               "Start of received response ----------------------------------------------------\n"
               "%s\n"
               "End of received response ------------------------------------------------------\n"
               "From host %s\n", requestBuf, ntop_buf);
        /* clang-format on */
        /* add thread pool job to handle request */
        if (data != NULL) {
            data->parser.msg.msg.length += (size_t)byteReceived;
            /* null-terminate */
            data->parser.msg.msg.buf[byteReceived] = 0;
            memcpy(&data->dest_addr, &__ss, sizeof(__ss));
            TPJobInit(&job, (start_routine)ssdp_event_handler_thread, data);
            TPJobSetFreeFunction(&job, free_ssdp_event_handler_data);
            TPJobSetPriority(&job, MED_PRIORITY);
            if (ThreadPoolAdd(&gRecvThreadPool, &job, NULL) != 0)
                free_ssdp_event_handler_data(data);
        }
        return 0;
    } else {
        free_ssdp_event_handler_data(data);
        return -1;
    }
}


int get_ssdp_sockets(MiniServerSockArray* out) {
    int retVal;

#ifdef COMPA_HAVE_CTRLPT_SSDP
    out->ssdpReqSock4 = INVALID_SOCKET;
    out->ssdpReqSock6 = INVALID_SOCKET;
    /* Create the IPv4 socket for SSDP REQUESTS */
    if (strlen(gIF_IPV4) > (size_t)0) {
        retVal = create_ssdp_sock_reqv4(&out->ssdpReqSock4);
        if (retVal != UPNP_E_SUCCESS)
            return retVal;
        /* For use by ssdp control point. */
        gSsdpReqSocket4 = out->ssdpReqSock4;
    } else
        out->ssdpReqSock4 = INVALID_SOCKET;
        /* Create the IPv6 socket for SSDP REQUESTS */
#ifdef UPNP_ENABLE_IPV6
    if (strlen(gIF_IPV6) > (size_t)0) {
        retVal = create_ssdp_sock_reqv6(&out->ssdpReqSock6);
        if (retVal != UPNP_E_SUCCESS) {
            UpnpCloseSocket(out->ssdpReqSock4);
            return retVal;
        }
        /* For use by ssdp control point. */
        gSsdpReqSocket6 = out->ssdpReqSock6;
    } else
        out->ssdpReqSock6 = INVALID_SOCKET;
#endif /* IPv6 */
#endif /* COMPA_HAVE_CTRLPT_SSDP */
    /* Create the IPv4 socket for SSDP */
    if (strlen(gIF_IPV4) > (size_t)0) {
        retVal = create_ssdp_sock_v4(&out->ssdpSock4);
        if (retVal != UPNP_E_SUCCESS) {
#ifdef COMPA_HAVE_CTRLPT_SSDP
            UpnpCloseSocket(out->ssdpReqSock4);
            UpnpCloseSocket(out->ssdpReqSock6);
#endif
            return retVal;
        }
    } else
        out->ssdpSock4 = INVALID_SOCKET;
#ifdef UPNP_ENABLE_IPV6
    /* Create the IPv6 socket for SSDP */
    if (strlen(gIF_IPV6) > (size_t)0) {
        retVal = create_ssdp_sock_v6(&out->ssdpSock6);
        if (retVal != UPNP_E_SUCCESS) {
            UpnpCloseSocket(out->ssdpSock4);
#ifdef COMPA_HAVE_CTRLPT_SSDP
            UpnpCloseSocket(out->ssdpReqSock4);
            UpnpCloseSocket(out->ssdpReqSock6);
#endif
            return retVal;
        }
    } else
        out->ssdpSock6 = INVALID_SOCKET;
    if (strlen(gIF_IPV6_ULA_GUA) > (size_t)0) {
        retVal = create_ssdp_sock_v6_ula_gua(&out->ssdpSock6UlaGua);
        if (retVal != UPNP_E_SUCCESS) {
            UpnpCloseSocket(out->ssdpSock4);
            UpnpCloseSocket(out->ssdpSock6);
#ifdef COMPA_HAVE_CTRLPT_SSDP
            UpnpCloseSocket(out->ssdpReqSock4);
            UpnpCloseSocket(out->ssdpReqSock6);
#endif
            return retVal;
        }
    } else
        out->ssdpSock6UlaGua = INVALID_SOCKET;
#endif /* UPNP_ENABLE_IPV6 */

    return UPNP_E_SUCCESS;
}
