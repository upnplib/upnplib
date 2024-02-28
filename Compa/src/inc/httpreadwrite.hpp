#ifdef COMPA_HAVE_WEBSERVER

#ifndef COMPA_GENLIB_NET_HTTP_HTTPREADWRITE_HPP
#define COMPA_GENLIB_NET_HTTP_HTTPREADWRITE_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-28
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
 ******************************************************************************/
// Last compare with ./pupnp source file on 2023-08-20, ver 1.14.17
/*!
 * \file
 * \brief Performs HTTP read and write messages.
 */

#include <config.hpp>
#include <httpparser.hpp>
#include <sock.hpp>

/*! timeout in secs. */
#define HTTP_DEFAULT_TIMEOUT 30

#if defined(_WIN32) || defined(DOXYGEN_RUN)
/// \brief Portable gmtime_r for Microsoft Windows.
tm* http_gmtime_r(const time_t* clock, tm* result);
#else
#define http_gmtime_r gmtime_r
#endif

/*!
 * \brief Set the cancel flag of the HttpGet handle.
 *
 * \returns
 *  - UPNP_E_SUCCESS       - On Success
 *  - UPNP_E_INVALID_PARAM - Invalid Parameter
 */
UPNPLIB_API int http_CancelHttpGet( //
    void* Handle                    ///< [in] Handle to HTTP get object.
);

/*!
 * \brief Validates URL.
 *
 * \returns
 *  - UPNP_E_INVALID_URL
 *  - UPNP_E_SUCCESS
 */
int http_FixUrl(        //
    uri_type* url,      ///< [in] URL to be validated and fixed.
    uri_type* fixed_url ///< [out] URL after being fixed.
);

/*!
 * \brief Parses URL and then validates URL.
 *
 * \returns
 *  - UPNP_E_INVALID_URL
 *  - UPNP_E_SUCCESS
 */
int http_FixStrUrl(     //
    const char* urlstr, ///< [in] Character string as a URL.
    size_t urlstrlen,   ///< [in] Length of the character string.
    uri_type* fixed_url ///< [out] Fixed and corrected URL.
);

/*!
 * \brief Gets destination address from URL and then connects to the
 * remote end.
 *
 * \returns
 * On success: Socket descriptor\n
 * On error:
 *  - UPNP_E_OUTOF_SOCKET
 *  - UPNP_E_SOCKET_CONNECT
 */
SOCKET http_Connect(           //
    uri_type* destination_url, ///< [in] URL containing destination information.
    uri_type* url              ///< [out] Fixed and corrected URL.
);

/*!
 * \brief Get the data on the socket and take actions based on the read data to
 * modify the parser objects buffer.
 *
 * If an error is reported while parsing the data, the error code is passed in
 * the http_error_code parameter.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error: UPNP_E_BAD_HTTPMSG
 */
UPNPLIB_API int http_RecvMessage( //
    SOCKINFO* info,               ///< [in] Socket information object.
    http_parser_t* parser,        ///< [out] HTTP parser object.
    http_method_t request_method, ///< [in] HTTP request method.
    int* timeout_secs,            ///< [in,out] time out.
    int* http_error_code          ///< [out] HTTP error code returned.
);

/*!
 * \brief Sends a message to the destination based on the format parameter.
 *
 * format (fmt) types:
 *  - 'f': arg  = "const char*" file name
 *  - 'b': arg1 = "const char*" mem_buffer, arg2 = "size_t" buffer length
 *  - 'I': arg  = "SendInstruction*" send instruction
 *
 * \note Sending from file (fmt = "If") always needs an instruction tag before
 * with at least SendInstruction.ReadSendSize set. Otherwise nothing is sent.\n
 * .ReadSendSize > 0: amount of bytes to send\n
 * .ReadSendSize = 0: nothing to send\n
 * .ReadSendSize < 0: send until end from data in file or until internal
 *                    sendbuffer size.\n
 * For example:
 \verbatim
    char *buf = "POST /xyz.cgi http/1.1\r\n\r\n";
    char *filename = "foo.dat";
    SendInstruction instruct;
    instruct.ReadSendSize = -1;
    int status = http_SendMessage(tcpsock, "Ibf",
        &instruct,              // arg pointer to send instruction
        buf, strlen(buf),       // args for memory buffer
        filename);              // arg for file
 \endverbatim
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_FILE_READ_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 *  - UPNP_E_SOCKET_ERROR
 */
UPNPLIB_API int http_SendMessage(
    SOCKINFO* info, ///< [in] Socket information object.
    int* TimeOut,   ///< [in,out] Time out value.
    const char*
        fmt, ///< [in] Pattern format to take actions upon (like printf()).
    ...      ///< [in] Variable argument list (like printf()).
);

/*!
 * \brief Initiates socket, connects to the remote host, sends a request and
 * waits for the response from the remote end.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_CONNECT
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_FILE_READ_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 *  - UPNP_E_BAD_HTTPMSG
 */
int http_RequestAndResponse(  //
    uri_type* destination,    /*!< [in] Destination URI object which contains
                               *   remote IP address among other elements. */
    const char* request,      ///< [in] Request to be sent.
    size_t request_length,    ///< [in] Length of the request.
    http_method_t req_method, ///< [in] HTTP Request method.
    int timeout_secs,         ///< [in] time out value.
    http_parser_t* response   ///< [in] Parser object to receive the repsonse.
);

/************************************************************************
 * return codes:
 *      0 -- success
 *      UPNP_E_OUTOF_MEMORY
 *      UPNP_E_TIMEDOUT
 *      UPNP_E_BAD_REQUEST
 *      UPNP_E_BAD_RESPONSE
 *      UPNP_E_INVALID_URL
 *      UPNP_E_SOCKET_READ
 *      UPNP_E_SOCKET_WRITE
 ************************************************************************/

/*!
 * \brief Download the document message and extract the document from the
 * message.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_INVALID_URL
 */
UPNPLIB_API int http_Download( //
    const char* url_str,       ///< [in] String as a URL.
    int timeout_secs,          ///< [in] Time out value.
    char** document, /*!< [out] Buffer to store the document extracted from the
                      *   donloaded message. */
    size_t* doc_length, ///< [out] Length of the extracted document.
    char* content_type  ///< [out] Type of content.
);

/*!
 * \brief Extracts information from the Handle to the HTTP get object.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_INVALID_PARAM
 */
UPNPLIB_API int http_HttpGetProgress(
    void* Handle,   ///< [in] Handle to the HTTP get object.
    size_t* length, ///< [out] Buffer to get the read and parsed data.
    size_t* total   ///< [out] Size of tge buffer passed.
);

/*!
 * \brief Opens a connection to the server.
 *
 * The SDK allocates the memory for the **handle**.
 * \note The calling application is responsible for freeing the memory allocated
 * for the **handle**.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_INVALID_PARAM - Either **url**, or **handle** is not a valid
 *                           pointer.
 *  - UPNP_E_INVALID_URL - The **url** is not a valid URL.
 *  - UPNP_E_OUTOF_MEMORY - Insufficient resources exist to download this file.
 *  - UPNP_E_SOCKET_ERROR - Error occured allocating a socket and resources or
 *                          an error occurred binding a socket.
 *  - UPNP_E_SOCKET_WRITE - An error or timeout occurred writing to a socket.
 *  - UPNP_E_SOCKET_CONNECT - An error occurred connecting a socket.
 *  - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 */
UPNPLIB_API int http_OpenHttpConnection(
    /*! [in] The URL which contains the host, and the scheme to make the
       connection. */
    const char* url_str,
    /*! [in,out] A pointer in which to store the handle for this connection.
     * This handle is required for futher operations over this connection. */
    void** Handle,
    /*! [in] The time out value sent with the request during which a response
     * is expected from the receiver, failing which, an error is reported.
     * If value is negative, timeout is infinite.\n
     * **This argument isn't used anymore** and only available for downstream
     * compatibility. It can be set to 0.*/
    int timeout);

/*!
 * \brief Makes a HTTP request using a connection previously created by
 * UpnpOpenHttpConnection().
 *
 * \note Trying to make another request while a request is already being
 * processed results in undefined behavior. It's up to the user to end a
 * previous request by calling UpnpEndHttpRequest().
 *
 *  \return An integer representing one of the following\n
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM - Either **url**, **handle** or **contentType** is
 *                           not a valid pointer.
 *  - UPNP_E_INVALID_URL - The **url** is not a valid URL.
 *  - UPNP_E_OUTOF_MEMORY - Insufficient resources exist to download this file.
 *  - UPNP_E_SOCKET_ERROR - Error occured allocating a socket and resources or
 *                          an error occurred binding a socket.
 *  - UPNP_E_SOCKET_WRITE - An error or timeout occurred writing to a socket.
 *  - UPNP_E_SOCKET_CONNECT - An error occurred connecting a socket.
 *  - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 */
UPNPLIB_API int http_MakeHttpRequest(
    /*! [in] The method to use to make the request. */
    Upnp_HttpMethod method,
    /*! [in] The URL to use to make the request. The URL should use the same
     * host and scheme used to create the connection. */
    const char* url_str,
    /*! [in] The handle to the connection. */
    void* Handle,
    /*! [in] Headers to be used for the request. Each header should be
     * terminated by a CRLF as specified in the HTTP specification. If NULL
     * then the default headers will be used. */
    UpnpString* headers,
    /*! [in] The media type of content being sent. Can be NULL. */
    const char* contentType,
    /*! [in] The length of the content being sent, in bytes. Set to
     * **UPNP_USING_CHUNKED** to use chunked encoding, or **UPNP_UNTIL_CLOSE**
     * to avoid specifying the content length to the server. In this case the
     * request is considered unfinished until the connection is closed. */
    int contentLength,
    /*! [in] The time out value sent with the request during which a response
     * is expected from the receiver, failing which, an error is reported. If
     * value is negative, timeout is infinite. */
    int timeout);

/*!
 * \brief Writes the content of a HTTP request initiated by a
 * UpnpMakeHttpRequest() call. The end of the content should be indicated by a
 * call to UpnpEndHttpRequest().
 *
 *  \return An integer representing one of the following\n
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM - Either \b handle, \b buf or \b size is not a valid
 *                           pointer.
 *  - UPNP_E_SOCKET_WRITE - An error or timeout occurred writing to a socket.
 *  - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 */
UPNPLIB_API int http_WriteHttpRequest(
    /*! [in] The handle of the connection created by the call to
     * UpnpOpenHttpConnection(). */
    void* Handle,
    /*! [in] The buffer containing date to be written. */
    char* buf,
    /*! [in] The size, in bytes of **buf**. */
    size_t* size,
    /*! [in] A timeout value sent with the request during which a response is
     * expected from the server, failing which, an error is reported. If
     * value is negative, timeout is infinite. */
    int timeout);

/*!
 * \brief Indicates the end of a HTTP request previously made by
 * UpnpMakeHttpRequest().
 *
 *  \return
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM - **handle** is not a valid pointer.
 *  - UPNP_E_OUTOF_MEMORY - Insufficient resources exist.
 *  - UPNP_E_SOCKET_ERROR - Error occured on handling a socket.
 *  - UPNP_E_SOCKET_WRITE - An error or timeout occurred writing to a socket.
 *  - UPNP_E_SOCKET_CONNECT - An error occurred connecting a socket.
 *  - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 */
UPNPLIB_API int http_EndHttpRequest(
    /*! [in] The handle to the connection. */
    void* Handle,
    /*! [in] The time out value sent with the request during which a response is
       expected from the receiver, failing which, an error is reported. If value
       is negative, timeout is infinite. */
    int timeout);

/*!
 * \brief Gets the response from the server using a connection previously
 * created by UpnpOpenHttpConnection().
 *
 * \note Memory for **contentType** is only valid until the next call to the
 * HTTP API for the same connection.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM - Either **handle**, **contentType**,
 *                           **contentLength** or **httpStatus** is not a valid
 *                           pointer.
 *  - UPNP_E_INVALID_URL - The **url** is not a valid URL.
 *  - UPNP_E_OUTOF_MEMORY - Insufficient resources exist to download this file.
 *  - UPNP_E_NETWORK_ERROR - A network error occurred.
 *  - UPNP_E_SOCKET_WRITE - An error or timeout occurred writing to a socket.
 *  - UPNP_E_SOCKET_READ - An error or timeout occurred reading from a socket.
 *  - UPNP_E_SOCKET_BIND - An error occurred binding a socket.
 *  - UPNP_E_SOCKET_CONNECT - An error occurred connecting a socket.
 *  - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 *  - UPNP_E_BAD_RESPONSE - A bad response was received from the remote server.
 */
UPNPLIB_API int http_GetHttpResponse(
    /*! [in] The handle of the connection created by the call to
       UpnpOpenHttpConnection(). */
    void* Handle,
    /*! [in] Headers sent by the server for the response. If nullptr then the
     * headers are not copied. */
    UpnpString* headers,
    /*! [out] A buffer to store the media type of the item. */
    char** contentType,
    /*! [out] A pointer to store the length of the item. */
    int* contentLength,
    /*! [out] The status returned on receiving a response message. */
    int* httpStatus,
    /*! [in] The time out value sent with the request during which a response
     * is expected from the server, failing which, an error is reported
     * back to the user. If value is negative, timeout is infinite. */
    int timeout);

/*!
 * \brief Reads the content of a response using a connection previously created
 * by UpnpOpenHttpConnection().
 *
 *  \return
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *  - UPNP_E_INVALID_PARAM - Either **handle**, **buf** or **size** is not a
 *                           valid pointer.
 *  - UPNP_E_BAD_RESPONSE - A bad response was received from the remote server.
 *  - UPNP_E_BAD_HTTPMSG - Either the request or response was in the incorrect
 *                         format.
 *  - UPNP_E_CANCELED - another thread called UpnpCancelHttpGet.
 *
 *  Note: In case of return values, the status code parameter of the passed
 *        in handle value may provide additional information on the return
 *        value.
 */
UPNPLIB_API int http_ReadHttpResponse(
    /*! [in] The handle of the connection created by the call to
       UpnpOpenHttpConnection(). */
    void* Handle,
    /*! [in,out] The buffer to store the read item. */
    char* buf,
    /*! [in,out] The size of the buffer to be read. */
    size_t* size,
    /*! [in] The time out value sent with the request during which a response is
       expected from the server, failing which, an error is reported back to the
       user. If value is negative, timeout is infinite. */
    int timeout);

/*!
 * \brief Closes the connection created with UpnpOpenHttpConnection() and frees
 * any memory associated with the connection.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS - The operation completed successfully.\n
 *  On error:
 *   - UPNP_E_INVALID_PARAM - **handle** is not a valid pointer.
 *   - UPNP_E_SOCKET_READ - An error or timeout occurred reading from a socket.
 *   - UPNP_E_OUTOF_SOCKET - Too many sockets are currently allocated.
 */
UPNPLIB_API int http_CloseHttpConnection(
    /*! [in] The handle of the connection to close, created by the call to
       UpnpOpenHttpPost(). */
    void* Handle);

/*!
 * \brief Generate a response message for the status query and send the status
 * response.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_SOCKET_WRITE
 *  - UPNP_E_TIMEDOUT
 */
UPNPLIB_API int http_SendStatusResponse(
    SOCKINFO* info,       ///< [in] Socket information object.
    int http_status_code, /*!< [in] Error code returned while making or sending
                           *   the response message. */
    int request_major_version, ///< [in] Request major version.
    int request_minor_version  ///< [in] Request minor version.
);

// clang-format off
/*!
 * \brief Generate an HTTP message based on the format that is specified in
 * the input parameters.
 *
\verbatim
Format types:
  'B':  arg = int status_code        -- appends content-length, content-type
                                        and HTML body for given code.
  'b':  arg1 = const char *buf;
        arg2 = size_t buf_length memory ptr
  'C':  (no args)                    -- appends a HTTP CONNECTION: close header
                                        depending on major, minor version.
  'c':  (no args)                    -- appends CRLF "\r\n"
  'D':  (no args)                    -- appends HTTP DATE: header
  'd':  arg = int number             -- appends decimal number
  'G':  arg = range information      -- add range header
  'h':  arg = off_t number           -- appends off_t number
  'K':  (no args)                    -- add chunky header
  'L':  arg = language information   -- add Content-Language header if Accept-
                                        Language header is not empty and if
                                        WEB_SERVER_CONTENT_LANGUAGE is not
                                        empty
  'N':  arg1 = off_t content_length  -- content-length header
  'q':  arg1 = http_method_t         -- request start line and HOST header
        arg2 = (uri_type *)
  'Q':  arg1 = http_method_t;        -- start line of request
        arg2 = char* url;
        arg3 = size_t url_length
  'R':  arg = int status_code        -- adds a response start line
  'S':  (no args)                    -- appends HTTP SERVER: header
  's':  arg = const char *           -- C_string
  'T':  arg = char * content_type;   -- format e.g: "text/html";
                                        content-type header
  't':  arg = time_t * gmt_time      -- appends time in RFC 1123 fmt
  'U':  (no args)                    -- appends HTTP USER-AGENT: header
  'X':  arg = const char *           -- useragent; "redsonic"
                                        HTTP X-User-Agent: useragent
\endverbatim
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_URL
 */
// clang-format on
int http_MakeMessage(
    membuffer* buf, ///< [in,out] Buffer with the contents of the message.
    int http_major_version, ///< [in] HTTP major version.
    int http_minor_version, ///< [in] HTTP minor version.
    const char* fmt,        ///< [in] Pattern format (like printf()).
    ...                     ///< [in] Variable Format arguments (like printf()(.
);

/*!
 * \brief Calculate HTTP response versions based on the request versions.
 */
void http_CalcResponseVersion(
    int request_major_vers,   ///< [in] Request major version.
    int request_minor_vers,   ///< [in] Request minor version.
    int* response_major_vers, ///< [in] Response mojor version.
    int* response_minor_vers  ///< [in] Response minor version.
);

/*!
 * \brief Makes the HTTP GET message, connects to the peer, sends the HTTP GET
 * request, gets the response and parses the response.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_INVALID_PARAM
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_BAD_RESPONSE
 */
UPNPLIB_API int http_OpenHttpGetEx(
    const char* url_str, ///< [in] String as a URL.
    void** Handle, ///< [in,out] Pointer to buffer to store HTTP post handle.
    char** contentType, ///< [in,out] Type of content.
    int* contentLength, ///< [out] length of content.
    int* httpStatus,    /*!< [out] HTTP status returned on receiving a response
                         *         message. */
    int lowRange,       ///< ???
    int highRange,      ///< ???
    int timeout         ///< [in] time out value.
);

/*!
 * \brief Returns the server information for the operating system.
 */
void get_sdk_info(
    char* info,     ///< [out] Buffer to store the operating system information.
    size_t infoSize ///< [in] Size of buffer.
);

#endif /* COMPA_GENLIB_NET_HTTP_HTTPREADWRITE_HPP */
#endif // COMPA_HAVE_WEBSERVER
