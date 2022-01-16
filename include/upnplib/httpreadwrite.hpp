// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-13

#ifndef INCLUDE_UPNPLIB_HTTPREADWRITE_HPP
#define INCLUDE_UPNPLIB_HTTPREADWRITE_HPP

namespace upnplib {

/*!
 * \brief Opens a connection to the server.
 *
 * The SDK allocates the memory for \b handle, the
 * application is responsible for freeing this memory.
 *
 *  \return An integer representing one of the following:
 *      \li \c UPNP_E_SUCCESS: The operation completed successfully.
 *      \li \c UPNP_E_INVALID_PARAM: Either \b url, or \b handle
 *              is not a valid pointer.
 *      \li \c UPNP_E_INVALID_URL: The \b url is not a valid
 *              URL.
 *      \li \c UPNP_E_OUTOF_MEMORY: Insufficient resources exist to
 *              download this file.
 *      \li \c UPNP_E_SOCKET_ERROR: Error occured allocating a socket and
 *		resources or an error occurred binding a socket.
 *      \li \c UPNP_E_SOCKET_WRITE: An error or timeout occurred writing
 *              to a socket.
 *      \li \c UPNP_E_SOCKET_CONNECT: An error occurred connecting a
 *              socket.
 *      \li \c UPNP_E_OUTOF_SOCKET: Too many sockets are currently
 *              allocated.
 */
EXPORT_SPEC int http_OpenHttpConnection(
    /*! [in] The URL which contains the host, and the scheme to make the
       connection. */
    const char* url,
    /*! [in,out] A pointer in which to store the handle for this connection.
     * This handle is required for futher operations over this connection.
     */
    void** handle,
    /*! [in] The time out value sent with the request during which a
     * response is expected from the receiver, failing which, an error is
     * reported. If value is negative, timeout is infinite. */
    int timeout);

} // namespace upnplib

#endif // INCLUDE_UPNPLIB_HTTPREADWRITE_HPP
