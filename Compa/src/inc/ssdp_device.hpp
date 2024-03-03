#ifdef COMPA_HAVE_DEVICE_SSDP

#ifndef COMPA_SSDP_DEVICE_HPP
#define COMPA_SSDP_DEVICE_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2011-2012 France Telecom All rights reserved.
 * Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-03
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
 * \brief Manage "Step 1: Discovery" of the UPnP+™ specification for UPnP
 * Devices with SSDP.
 *
 * \ingroup compa-Discovery
 */

#include <ssdp_common.hpp>


/*!
 * \brief Handles the search request.
 *
 * It does the sanity checks of the request and then schedules a thread to send
 * a random time reply (random within maximum time given by the control point to
 * reply).
 *
 * \note Only available when the SSDP option for Devices was enabled on
 * compiling the library.
 */
void ssdp_handle_device_request(
    /*! [in] */
    http_message_t* hmsg,
    /*! [in] */
    struct sockaddr_storage* dest_addr);


/*! @{
 * \ingroup SSDP-device_functions */

/*!
 * \brief Sends SSDP advertisements, replies and shutdown messages.
 *
 * \note This function is only available when the Device option was enabled on
 * compiling the library.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  There are several messages on stderr if DEBUG is enabled.\n
 *  On error:
 *  - UPNP_E_INVALID_HANDLE
 */
UPNPLIB_API int AdvertiseAndReply(
    /*! [in] -1 = Send shutdown, 0 = send reply, 1 = Send Advertisement. */
    int AdFlag,
    /*! [in] Device handle. */
    UpnpDevice_Handle Hnd,
    /*! [in] Search type for sending replies. */
    enum SsdpSearchType SearchType,
    /*! [in] Destination address. */
    struct sockaddr* DestAddr,
    /*! [in] Device type. */
    char* DeviceType,
    /*! [in] Device UDN. */
    char* DeviceUDN,
    /*! [in] Service type. */
    char* ServiceType,
    /*! [in] Advertisement age. */
    int Exp);

/*!
 * \brief Wrapper function to reply the search request coming from the
 * control point.
 */
void advertiseAndReplyThread(
    /*! [in] Structure containing the search request. */
    void* data);

/*!
 * \brief Creates the device advertisement request.
 *
 * Create it based on the input parameter, and send it to the multicast channel.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int DeviceAdvertisement(
    /* [in] type of the device. */
    char* DevType,
    /* [in] flag to indicate if the device is root device. */
    int RootDev,
    /* [in] UDN. */
    char* Udn,
    /* [in] Location URL. */
    char* Location,
    /* [in] Service duration in sec. */
    int Duration,
    /* [in] Device address family. */
    int AddressFamily,
    /* [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /* [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /* [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates the reply packet and send it to the Control Point addesss.
 *
 * Creates the reply packet based on the input parameter, and send it to the
 * Control Point addesss given in its input parameter DestAddr.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int SendReply(
    /*! [in] destination IP address. */
    struct sockaddr* DestAddr,
    /*! [in] Device type. */
    char* DevType,
    /*! [in] 1 means root device 0 means embedded device. */
    int RootDev,
    /*! [in] Device UDN. */
    char* Udn,
    /*! [in] Location of Device description document. */
    char* Location,
    /*! [in] Life time of this device. */
    int Duration,
    /*! [in] */
    int ByType,
    /*! [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /*! [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /*! [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates the reply packet and send it to the Control Point address.
 *
 * Creates the reply packet based on the input parameter, and send it to the
 * Control Point address given in its input parameter DestAddr.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int DeviceReply(
    /*! [in] destination IP address. */
    struct sockaddr* DestAddr,
    /*! [in] Device type. */
    char* DevType,
    /*! [in] 1 means root device 0 means embedded device. */
    int RootDev,
    /*! [in] Device UDN. */
    char* Udn,
    /*! [in] Location of Device description document. */
    char* Location,
    /*! [in] Life time of this device. */
    int Duration,
    /*! [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /*! [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /*! [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates the advertisement packet and send it to the multicast channel.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int ServiceAdvertisement(
    /*! [in] Device UDN. */
    char* Udn,
    /*! [in] Service Type. */
    char* ServType,
    /*! [in] Location of Device description document. */
    char* Location,
    /*! [in] Life time of this device. */
    int Duration,
    /*! [in] Device address family. */
    int AddressFamily,
    /*! [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /*! [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /*! [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates the advertisement packet and send it to the multicast channel.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int ServiceReply(
    /*! [in] */
    struct sockaddr* DestAddr,
    /*! [in] Service Type. */
    char* ServType,
    /*! [in] Device UDN. */
    char* Udn,
    /*! [in] Location of Device description document. */
    char* Location,
    /*! [in] Life time of this device. */
    int Duration,
    /*! [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /*! [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /*! [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates a HTTP service shutdown request packet and sends it to the
 * multicast channel through RequestHandler.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int ServiceShutdown(
    /* [in] Device UDN. */
    char* Udn,
    /* [in] Service Type. */
    char* ServType,
    /* [in] Location of Device description document. */
    char* Location,
    /* [in] Service duration in sec. */
    int Duration,
    /* [in] Device address family. */
    int AddressFamily,
    /* [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /* [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /* [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/*!
 * \brief Creates a HTTP device shutdown request packet and send it to the
 * multicast channel through RequestHandler.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_NETWORK_ERROR
 *  - UPNP_E_SOCKET_WRITE
 */
int DeviceShutdown(
    /*! [in] Device Type. */
    char* DevType,
    /*! [in] 1 means root device. */
    int RootDev,
    /*! [in] Device UDN. */
    char* Udn,
    /*! [in] Location URL. */
    char* Location,
    /*! [in] Device duration in sec. */
    int Duration,
    /*! [in] Device address family. */
    int AddressFamily,
    /*! [in] PowerState as defined by UPnP Low Power. */
    int PowerState,
    /*! [in] SleepPeriod as defined by UPnP Low Power. */
    int SleepPeriod,
    /*! [in] RegistrationState as defined by UPnP Low Power. */
    int RegistrationState);

/// @} SSDP Device Functions

#endif // COMPA_SSDP_DEVICE_HPP
#endif // COMPA_HAVE_DEVICE_SSDP
