#ifndef UPNPLIB_GLOBAL_HPP
#define UPNPLIB_GLOBAL_HPP
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-25

#include "upnplib/visibility.hpp" // for UPNPLIB_API

namespace upnplib {

// Version of the upnplib library
UPNPLIB_API extern char library_version[];

/*!
 * \name Error codes
 *
 * The functions in the SDK API can return a variety of error
 * codes to describe problems encountered during execution.  This section
 * lists the error codes and provides a brief description of what each error
 * code means.  Refer to the documentation for each function for a
 * description of what an error code means in that context.
 *
 * @{
 */

/*!
 * \brief The operation completed successfully.
 *
 * For asynchronous functions, this only means that the packet generated by
 * the operation was successfully transmitted on the network.  The result of
 * the entire operation comes as part of the callback for that operation.
 */
#define UPNP_E_SUCCESS 0

/*!
 * \brief The handle passed to a function is not a recognized as a valid handle.
 */
#define UPNP_E_INVALID_HANDLE -100

/*!
 * \brief One or more of the parameters passed to the function is not valid.
 *
 * Refer to the documentation for each function for more information on the
 * valid ranges of the parameters.
 */
#define UPNP_E_INVALID_PARAM -101

/*!
 * \brief The SDK does not have any more space for additional handles.
 *
 * The SDK allocates space for only a few handles in order to conserve memory.
 */
#define UPNP_E_OUTOF_HANDLE -102

#define UPNP_E_OUTOF_CONTEXT -103

/*!
 * \brief Not enough resources are currently available to complete the
 * operation.
 *
 * Most operations require some free memory in order to complete their work.
 */
#define UPNP_E_OUTOF_MEMORY -104

/*!
 * \brief The SDK has already been initialized.
 *
 * The SDK needs to be initialied only once per process. Any additional
 * initialization attempts simply return this error with no other ill effects.
 */
#define UPNP_E_INIT -105

#define UPNP_E_BUFFER_TOO_SMALL -106

/*!
 * \brief The description document passed to \b UpnpRegisterRootDevice,
 * \b UpnpRegisterRootDevice2 \b UpnpRegisterRootDevice3 or
 * \b UpnpRegisterRootDevice4 is invalid.
 */
#define UPNP_E_INVALID_DESC -107

/*!
 * \brief An URL passed into the function is invalid.
 *
 * The actual cause is function specific, but in general, the URL itself
 * might be malformed (e.g. have invalid characters in it) or the host might
 * be unreachable.
 */
#define UPNP_E_INVALID_URL -108

#define UPNP_E_INVALID_SID -109

#define UPNP_E_INVALID_DEVICE -110

/*!
 * \brief The device ID/service ID pair does not refer to a valid service.
 *
 * Returned only by \b UpnpNotify, \b UpnpNotifyExt, \b UpnpAcceptSubscription,
 * and \b UpnpAcceptSubscriptionExt.
 */
#define UPNP_E_INVALID_SERVICE -111

/*!
 * \brief The response received from the remote side of a connection is not
 * correct for the protocol.
 *
 * This applies to the GENA, SOAP, and HTTP protocols.
 */
#define UPNP_E_BAD_RESPONSE -113

#define UPNP_E_BAD_REQUEST -114

/*!
 * \brief The SOAP action message is invalid.
 *
 * This can be because the DOM document passed to the function was malformed or
 * the action message is not correct for the given action.
 */
#define UPNP_E_INVALID_ACTION -115

/*!
 * \brief \b UpnpInit2 has not been called, or \b UpnpFinish has already been
 * called.
 *
 * None of the API functions operate until \b UpnpInit2 successfully completes.
 */
#define UPNP_E_FINISH -116

/*!
 * \brief \b UpnpInit2 cannot complete.
 *
 * The typical reason is failure to allocate sufficient resources.
 */
#define UPNP_E_INIT_FAILED -117

/*!
 * \brief The URL passed into a function is too long.
 *
 * The SDK limits URLs to 180 characters in length.
 */
#define UPNP_E_URL_TOO_BIG -118

/*!
 * \brief The HTTP message contains invalid message headers.
 *
 * The error always refers to the HTTP message header received from the remote
 * host.  The main areas where this occurs are in SOAP control messages (e.g.
 * \b UpnpSendAction), GENA subscription message (e.g. \b UpnpSubscribe),
 * GENA event notifications (e.g. \b UpnpNotify), and HTTP transfers (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_BAD_HTTPMSG -119

/*!
 * \brief A client or a device is already registered.
 *
 * The SDK currently has a limit of one registered client and one registered
 * device per process.
 */
#define UPNP_E_ALREADY_REGISTERED -120

/*!
 * \brief The interface provided to \b UpnpInit2 is unknown or does not have a
 * valid IPv4 or IPv6 address configured.
 */
#define UPNP_E_INVALID_INTERFACE -121

/*!
 * \brief A network error occurred.
 *
 * It is the generic error code for network problems that are not covered under
 * one of the more specific error codes.  The typical meaning is the SDK failed
 * to read the local IP address or had problems configuring one of the sockets.
 */
#define UPNP_E_NETWORK_ERROR -200

/*!
 * \brief An error happened while writing to a socket.
 *
 * This occurs in any function that makes network connections, such as discovery
 * (e.g. \b UpnpSearchAsync or \b UpnpSendAdvertisement), control (e.g.
 * \b UpnpSendAction), eventing (e.g. \b UpnpNotify), and HTTP functions (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_SOCKET_WRITE -201

/*!
 * \brief An error happened while reading from a socket.
 *
 * This occurs in any function that makes network connections, such as discovery
 * (e.g. \b UpnpSearchAsync or \b UpnpSendAdvertisement), control (e.g.
 * \b UpnpSendAction), eventing (e.g. \b UpnpNotify), and HTTP functions (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_SOCKET_READ -202

/*!
 * \brief The SDK had a problem binding a socket to a network interface.
 *
 * This occurs in any function that makes network connections, such as discovery
 * (e.g. \b UpnpSearchAsync or \b UpnpSendAdvertisement), control (e.g.
 * \b UpnpSendAction), eventing (e.g. \b UpnpNotify), and HTTP functions (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_SOCKET_BIND -203

/*!
 * \brief The SDK had a problem connecting to a remote host.
 *
 * This occurs in any function that makes network connections, such as discovery
 * (e.g. \b UpnpSearchAsync or \b UpnpSendAdvertisement), control (e.g.
 * \b UpnpSendAction), eventing (e.g. \b UpnpNotify), and HTTP functions (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_SOCKET_CONNECT -204

/*!
 * \brief The SDK cannot create any more sockets.
 *
 * This occurs in any function that makes network connections, such as discovery
 * (e.g. \b UpnpSearchAsync or \b UpnpSendAdvertisement), control (e.g.
 * \b UpnpSendAction), eventing (e.g. \b UpnpNotify), and HTTP functions (e.g.
 * \b UpnpDownloadXmlDoc).
 */
#define UPNP_E_OUTOF_SOCKET -205

/*!
 * \brief The SDK had a problem setting the socket to listen for incoming
 * connections.
 *
 * This error only happens during initialization (i.e. \b UpnpInit2).
 */
#define UPNP_E_LISTEN -206

/*!
 * \brief Too much time elapsed before the required number of bytes were sent
 * or received over a socket.
 *
 * This error can be returned by any function that performs network operations.
 */
#define UPNP_E_TIMEDOUT -207

/*!
 * \brief Generic socket error code for conditions not covered by other error
 * codes.
 *
 * This error can be returned by any function that performs network operations.
 */
#define UPNP_E_SOCKET_ERROR -208

#define UPNP_E_FILE_WRITE_ERROR -209

/*! \brief The operation was canceled.
 *
 * This error can be returned by any function that allows for external
 * cancelation.
 */
#define UPNP_E_CANCELED -210

#define UPNP_E_EVENT_PROTOCOL -300

/*!
 * \brief A subscription request was rejected from the remote side.
 */
#define UPNP_E_SUBSCRIBE_UNACCEPTED -301

/*!
 * \brief An unsubscribe request was rejected from the remote side.
 */
#define UPNP_E_UNSUBSCRIBE_UNACCEPTED -302

/*!
 * \brief The remote host did not accept the notify sent from the local device.
 */
#define UPNP_E_NOTIFY_UNACCEPTED -303

/*!
 * \brief One or more of the parameters passed to a function is invalid.
 *
 * Refer to the individual function descriptions for the acceptable ranges for
 * parameters.
 */
#define UPNP_E_INVALID_ARGUMENT -501

/*!
 * \brief The filename passed to one of the device registration functions was
 * not found or was not accessible.
 */
#define UPNP_E_FILE_NOT_FOUND -502

/*!
 * \brief An error happened while reading a file.
 */
#define UPNP_E_FILE_READ_ERROR -503

/*!
 * \brief The file name of the description document passed to
 * \b UpnpRegisterRootDevice2 does not end in ".xml".
 */
#define UPNP_E_EXT_NOT_XML -504

#define UPNP_E_NO_WEB_SERVER -505
#define UPNP_E_OUTOF_BOUNDS -506

/*!
 * \brief The response to a SOAP request did not contain the required XML
 * constructs.
 */
#define UPNP_E_NOT_FOUND -507

/*!
 * \brief Generic error code for internal conditions not covered by other
 * error codes.
 */
#define UPNP_E_INTERNAL_ERROR -911

/* SOAP-related error codes */
#define UPNP_SOAP_E_INVALID_ACTION 401
#define UPNP_SOAP_E_INVALID_ARGS 402
#define UPNP_SOAP_E_OUT_OF_SYNC 403
#define UPNP_SOAP_E_INVALID_VAR 404
#define UPNP_SOAP_E_ACTION_FAILED 501

/* @} ErrorCodes */

} // namespace upnplib

#endif // UPNPLIB_GLOBAL_HPP
