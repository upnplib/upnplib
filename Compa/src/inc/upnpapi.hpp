#ifndef COMPA_UPNPAPI_HPP
#define COMPA_UPNPAPI_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2011-2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-01-27
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
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
 ******************************************************************************/
// Last compare with pupnp original source file on 2023-07-08, ver 1.14.17
/*!
 * \file
 * \brief Inititalize the compatible library before it can be used.
 */

#include <VirtualDir.hpp> /* for struct VirtualDirCallbacks */
#include <client_table.hpp>

/// MAX_INTERFACES
#define MAX_INTERFACES 256

/// DEV_LIMIT
#define DEV_LIMIT 200

/// DEFAULT_MX
#define DEFAULT_MX 5

/// DEFAULT_MAXAGE
#define DEFAULT_MAXAGE 1800

/// DEFAULT_SOAP_CONTENT_LENGTH
#define DEFAULT_SOAP_CONTENT_LENGTH 16000
/// MAX_SOAP_CONTENT_LENGTH
#define MAX_SOAP_CONTENT_LENGTH (size_t)32000

/// NUM_HANDLE
#define NUM_HANDLE 200

extern size_t g_maxContentLength;
extern int g_UpnpSdkEQMaxLen;
extern int g_UpnpSdkEQMaxAge;

/// UPNP_TIMEOUT
#define UPNP_TIMEOUT 30

/// Specifies if a device, or client has to be handled for a connection.
typedef enum { HND_INVALID = -1, HND_CLIENT, HND_DEVICE } Upnp_Handle_Type;


/// \brief Data to be stored in handle table for Handle Info.
struct Handle_Info {
    Upnp_Handle_Type HType; ///< Handle Type
    Upnp_FunPtr Callback;   ///< Callback function pointer.
    char* Cookie;           ///< ???
    int aliasInstalled;     ///< 0 = not installed; otherwise installed.

#if defined(INCLUDE_DEVICE_APIS) || defined(DOXYGEN_RUN)
    /// \name Following attributes are only valid with managing a device.
    /// @{
    char DescURL[LINE_SIZE];      ///< URL for the use of SSDP.
    char LowerDescURL[LINE_SIZE]; /*!< \brief URL for the use of SSDP when
                                   * answering to legacy CPs (CP searching for a
                                   * v1 when the device is v2). */
    char DescXML[LINE_SIZE];      ///< XML file path for device description.
    int MaxAge;                   ///< Advertisement timeout.
    int PowerState;               ///< Power State as defined by UPnP Low Power.
    int SleepPeriod;       ///< Sleep Period as defined by UPnP Low Power.
    int RegistrationState; ///< Registration State as defined by UPnP Low Power.
    IXML_Document*
        DescDocument;          ///< Description parsed in terms of DOM document.
    IXML_NodeList* DeviceList; ///< List of devices in the description document.
    IXML_NodeList*
        ServiceList; ///< List of services in the description document.
    service_table
        ServiceTable;     ///< Table holding subscriptions and URL information.
    int MaxSubscriptions; ///< ???
    int MaxSubscriptionTimeOut; ///< ???
    int DeviceAf;               ///< Address family: AF_INET6 or AF_INET.
    /// @}
#endif

#if defined(INCLUDE_CLIENT_APIS) || defined(DOXYGEN_RUN)
    /// \name Following attributes are only valid with managing a client.
    /// @{
    GenlibClientSubscription* ClientSubList; ///< Client subscription list.
    LinkedList SsdpSearchList;               ///< Active SSDP searches.
    /// @}
#endif
};

extern ithread_rwlock_t GlobalHndRWLock;

/*!
 * \brief Get handle information.
 *
 * \return HND_DEVICE, HND_CLIENT, HND_INVALID
 */
Upnp_Handle_Type GetHandleInfo(
    /*! [in] handle number (table index for the client handle structure). */
    int Hnd,
    /*! [out] handle structure passed by this function. */
    struct Handle_Info** HndInfo);

/// HandleLock
#define HandleLock() HandleWriteLock()

/// HandleWriteLock
#define HandleWriteLock()                                                      \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Trying a write lock\n");   \
    ithread_rwlock_wrlock(&GlobalHndRWLock);                                   \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Write lock acquired\n");

/// HandleReadLock
#define HandleReadLock()                                                       \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Trying a read lock\n");    \
    ithread_rwlock_rdlock(&GlobalHndRWLock);                                   \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Read lock acquired\n");

/// HandleUnlock
#define HandleUnlock()                                                         \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Trying Unlock\n");         \
    ithread_rwlock_unlock(&GlobalHndRWLock);                                   \
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__, "Unlocked rwlock\n");

/*!
 * \brief Get client handle info.
 *
 * \note The logic around the use of this function should be revised.
 *
 * \return HND_CLIENT, HND_INVALID
 */
Upnp_Handle_Type GetClientHandleInfo(
    /*! [in] client handle pointer (key for the client handle structure). */
    int* client_handle_out,
    /*! [out] Client handle structure passed by this function. */
    struct Handle_Info** HndInfo);
/*!
 * \brief Retrieves the device handle and information of the first device of
 *  the address family specified. The search begins at the 'start' index,
 * which should be 0 for the first call, then the last successful value
 * returned. This allows listing all entries for the address family.
 *
 * \return HND_DEVICE or HND_INVALID
 */
Upnp_Handle_Type GetDeviceHandleInfo(
    /*! [in] place to start the search (i.e. last value returned). */
    UpnpDevice_Handle start,
    /*! [in] Address family. */
    int AddressFamily,
    /*! [out] Device handle pointer. */
    int* device_handle_out,
    /*! [out] Device handle structure passed by this function. */
    struct Handle_Info** HndInfo);

/*!
 * \brief Retrieves the device handle and information of the first device of
 *  the address family specified, with a service having a controlURL or
 *  eventSubURL matching the path.
 *
 * \return HND_DEVICE or HND_INVALID
 */
Upnp_Handle_Type GetDeviceHandleInfoForPath(
    /*! The Uri path. */
    const char* path,
    /*! [in] Address family. */
    int AddressFamily,
    /*! [out] Device handle pointer. */
    int* device_handle_out,
    /*! [out] Device handle structure passed by this function. */
    struct Handle_Info** HndInfo,
    /*! [out] Service info for found path. */
    service_info** serv_info);

UPNPLIB_API extern char gIF_NAME[LINE_SIZE];
UPNPLIB_API extern char gIF_IPV4[INET_ADDRSTRLEN];
UPNPLIB_API extern char gIF_IPV4_NETMASK[INET_ADDRSTRLEN];
UPNPLIB_API extern char gIF_IPV6[INET6_ADDRSTRLEN];
UPNPLIB_API extern unsigned gIF_IPV6_PREFIX_LENGTH;

UPNPLIB_API extern char gIF_IPV6_ULA_GUA[INET6_ADDRSTRLEN];
UPNPLIB_API extern unsigned gIF_IPV6_ULA_GUA_PREFIX_LENGTH;

UPNPLIB_API extern unsigned gIF_INDEX;

UPNPLIB_API extern unsigned short LOCAL_PORT_V4;
UPNPLIB_API extern unsigned short LOCAL_PORT_V6;
UPNPLIB_API extern unsigned short LOCAL_PORT_V6_ULA_GUA;

/*! NLS uuid. */
extern Upnp_SID gUpnpSdkNLSuuid;

extern TimerThread gTimerThread;
UPNPLIB_API extern ThreadPool gRecvThreadPool;
extern ThreadPool gSendThreadPool;
UPNPLIB_API extern ThreadPool gMiniServerThreadPool;

/// UpnpFunName
typedef enum {
    SUBSCRIBE,
    UNSUBSCRIBE,
    DK_NOTIFY,
    QUERY,
    ACTION,
    STATUS,
    DEVDESCRIPTION,
    SERVDESCRIPTION,
    MINI,
    RENEW
} UpnpFunName;

/// \brief UpnpNonblockParam
struct UpnpNonblockParam {
    /// @{
    /// \brief %UpnpNonblockParam
    UpnpFunName FunName;
    int Handle;
    int TimeOut;
    char VarName[NAME_SIZE];
    char NewVal[NAME_SIZE];
    char DevType[NAME_SIZE];
    char DevId[NAME_SIZE];
    char ServiceType[NAME_SIZE];
    char ServiceVer[NAME_SIZE];
    char Url[NAME_SIZE];
    Upnp_SID SubsId;
    char* Cookie;
    Upnp_FunPtr Fun;
    IXML_Document* Header;
    IXML_Document* Act;
    struct DevDesc* Devdesc;
    /// @}
};

extern virtualDirList* pVirtualDirList;
extern struct VirtualDirCallbacks virtualDirCallback;

/// Possible status of the internal webserver.
typedef enum { WEB_SERVER_DISABLED, WEB_SERVER_ENABLED } WebServerState;

/// E_HTTP_SYNTAX
#define E_HTTP_SYNTAX -6

/*!
 * \brief Retrieve interface information and keep it in global variables.
 * If NULL, we'll find the first suitable interface for operation.
 *
 * The interface must fulfill these requirements:
 * \li Be UP.
 * \li Not be LOOPBACK.
 * \li Support MULTICAST.
 * \li Have a valid IPv4 or IPv6 address.
 *
 * We'll retrieve the following information from the interface:
 * \li gIF_NAME -> Interface name (by input or found).
 * \li gIF_IPV4 -> IPv4 address (if any).
 * \li gIF_IPV6 -> IPv6 address (if any).
 * \li gIF_IPV6_ULA_GUA -> ULA or GUA IPv6 address (if any)
 * \li gIF_INDEX -> Interface index number.
 *
 * \return UPNP_E_SUCCESS on success.
 */
UPNPLIB_API int UpnpGetIfInfo(
    /*! [in] Interface name (can be NULL). */
    const char* IfName);

/// UpnpThreadDistribution
void UpnpThreadDistribution(struct UpnpNonblockParam* Param);

/*!
 * \brief This function is a timer thread scheduled by UpnpSendAdvertisement
 * to the send advetisement again.
 */
void AutoAdvertise(
    /*! [in] Information provided to the thread. */
    void* input);

/*!
 * \brief Print handle info.
 *
 * \return UPNP_E_SUCCESS if successful, otherwise returns appropriate error.
 */
int PrintHandleInfo(
    /*! [in] Handle index. */
    UpnpClient_Handle Hnd);

/*! */
extern WebServerState bWebServerState;

/*! */
UPNPLIB_API extern WebCallback_HostValidate gWebCallback_HostValidate;

/*! */
UPNPLIB_API extern void* gWebCallback_HostValidateCookie;

/*! */
UPNPLIB_API extern int gAllowLiteralHostRedirection;

#endif /* COMPA_UPNPAPI_HPP */
