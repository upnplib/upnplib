#ifndef COMPA_SSDP_CTRLPT_HPP
#define COMPA_SSDP_CTRLPT_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2011-2012 France Telecom All rights reserved.
 * Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-21
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
 * \brief Manage "Step 1: Discovery" of the UPnP+™ specification for Control
 * Points with SSDP.
 *
 * \ingroup compa-Discovery
 */

#include <httpparser.hpp>

#ifndef COMPA_INTERNAL_CONFIG_HPP
#error "No or wrong config.hpp header file included."
#endif

#ifdef INCLUDE_CLIENT_APIS

/*! @{
 * \ingroup SSDP-ctrlpt_functions */

/*!
 * \brief This function handles the ssdp messages from the devices.
 *
 * These messages includes the search replies, advertisement of device coming
 * alive and bye byes.
 */
UPNPLIB_API void ssdp_handle_ctrlpt_msg(
    /*! [in] SSDP message from the device. */
    http_message_t* hmsg,
    /*! [in] Address of the device. */
    sockaddr_storage* dest_addr,
    /*! [in] timeout kept by the control point while sending search message.
       Only in search reply. */
    int timeout);

/*!
 * \brief Creates and send the search request for a specific URL.
 *
 * This function implements the search request of the discovery phase.
 * A M-SEARCH request is sent on the SSDP channel for both IPv4 and
 * IPv6 addresses. The search target(ST) is required and must be one of
 * the following:
 *     - "ssdp:all" : Search for all devices and services.
 *     - "ssdp:rootdevice" : Search for root devices only.
 *     - "uuid:<device-uuid>" : Search for a particular device.
 *     - "urn:schemas-upnp-org:device:<deviceType:v>"
 *     - "urn:schemas-upnp-org:service:<serviceType:v>"
 *     - "urn:<domain-name>:device:<deviceType:v>"
 *     - "urn:<domain-name>:service:<serviceType:v>"
 *
 * \returns
 *  On success: **1**\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_INTERNAL_ERROR
 *  - UPNP_E_INVALID_ARGUMENT
 *  - UPNP_E_BUFFER_TOO_SMALL
 */
UPNPLIB_API int SearchByTarget(
    /*! [in] The handle of the client performing the search. */
    int Hnd,
    /*! [in] Number of seconds to wait, to collect all the responses. */
    int Mx,
    /*! [in] Search target. */
    char* St,
    /*! [in] Cookie provided by control point application. This cokie will be
       returned to application in the callback. */
    void* Cookie);

/*!
 * \brief Creates the SSDP IPv4 socket to be used by the control point.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET
 */
int create_ssdp_sock_reqv4(
    /*! [out] SSDP IPv4 request socket to be created. */
    SOCKET* ssdpReqSock);

/*!
 * \brief Creates the SSDP IPv6 socket to be used by the control point.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_SOCKET
 */
int create_ssdp_sock_reqv6(
    /*! [out] SSDP IPv6 request socket to be created. */
    SOCKET* ssdpReqSock);

/// @} // SSDP Control Point Functions

#endif // INCLUDE_CLIENT_APIS
#endif // COMPA_SSDP_CTRLPT_HPP
