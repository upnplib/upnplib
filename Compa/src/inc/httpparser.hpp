#ifndef COMPA_GENLIB_NET_HTTP_HTTPPARSER_HPP
#define COMPA_GENLIB_NET_HTTP_HTTPPARSER_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-01-29
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
// Last verify with this project pupnp source file on 2023-07-20, ver 1.14.17
/*!
 * \file
 * \brief Functions to parse UPnP messages like requests and responses.
 */

#include <LinkedList.hpp>
#include <membuffer.hpp>
#include <upnputil.hpp>
#include <uri.hpp>

/*! \name Constants that define the read method.
 * @{
 */
/// \brief Defines read method
#define ENTREAD_DETERMINE_READ_METHOD 1
#define ENTREAD_USING_CLEN 2
#define ENTREAD_USING_CHUNKED 3
#define ENTREAD_UNTIL_CLOSE 4
#define ENTREAD_CHUNKY_BODY 5
#define ENTREAD_CHUNKY_HEADERS 6
/// @}

/// Type of a parser position
enum parser_pos_t {
    POS_REQUEST_LINE,  ///< Position request line
    POS_RESPONSE_LINE, ///< Position response line
    POS_HEADERS,       ///< Position headers
    POS_ENTITY,        ///< Position entity
    POS_COMPLETE       ///< Position complete
};

/// \brief Structure of a scanner object.
struct scanner_t {
    /// Raw http message.
    membuffer* msg;
    /// Current position in buffer.
    size_t cursor;
    /*! \brief Completeness of 'msg'.
     * - Set to 1 if the entire message is loaded in 'msg'.
     * - Set to 0 if only a partial message is in 'msg'.
     * - Default is 0. */
    int entire_msg_loaded;
};

/*! \brief Method in a HTTP request. **For a Warning see**
 * \warning The enum values of the standard HTTP method should match
 * those of Upnp_HttpMethod enum defined in Compa/inc/upnp.hpp */
enum http_method_t {
    HTTPMETHOD_PUT = UPNP_HTTPMETHOD_PUT,
    HTTPMETHOD_DELETE = UPNP_HTTPMETHOD_DELETE,
    HTTPMETHOD_GET = UPNP_HTTPMETHOD_GET,
    HTTPMETHOD_HEAD = UPNP_HTTPMETHOD_HEAD,
    HTTPMETHOD_POST = UPNP_HTTPMETHOD_POST,
    HTTPMETHOD_MPOST,
    HTTPMETHOD_SUBSCRIBE,
    HTTPMETHOD_UNSUBSCRIBE,
    HTTPMETHOD_NOTIFY,
    HTTPMETHOD_MSEARCH,
    HTTPMETHOD_UNKNOWN,
    SOAPMETHOD_POST,
    HTTPMETHOD_SIMPLEGET
};

/*! \name Different types of HTTP headers.
 * @{
 */
/// Type of a HTTP header
#define HDR_UNKNOWN -1
#define HDR_CACHE_CONTROL 1
#define HDR_CALLBACK 2
#define HDR_CONTENT_LENGTH 3
#define HDR_CONTENT_TYPE 4
#define HDR_DATE 5
#define HDR_EXT 6
#define HDR_HOST 7
/*define HDR_IF_MODIFIED_SINCE		8 */
/*define HDR_IF_UNMODIFIED_SINCE	9 */
/*define HDR_LAST_MODIFIED		10 */
#define HDR_LOCATION 11
#define HDR_MAN 12
#define HDR_MX 13
#define HDR_NT 14
#define HDR_NTS 15
#define HDR_SERVER 16
#define HDR_SEQ 17
#define HDR_SID 18
#define HDR_SOAPACTION 19
#define HDR_ST 20
#define HDR_TIMEOUT 21
#define HDR_TRANSFER_ENCODING 22
#define HDR_USN 23
#define HDR_USER_AGENT 24
/// @}
/*! \name Adding new header definitions
 * @{
 */
/// Header definition
#define HDR_ACCEPT 25
#define HDR_ACCEPT_ENCODING 26
#define HDR_ACCEPT_CHARSET 27
#define HDR_ACCEPT_LANGUAGE 28
#define HDR_ACCEPT_RANGE 29
#define HDR_CONTENT_ENCODING 30
#define HDR_CONTENT_LANGUAGE 31
#define HDR_CONTENT_LOCATION 32
#define HDR_CONTENT_RANGE 33
#define HDR_IF_RANGE 34
#define HDR_RANGE 35
#define HDR_TE 36
/// @}

/*! \brief Status of parsing. */
enum parse_status_t {
    /*! msg was parsed successfully. */
    PARSE_SUCCESS = 0,
    /*! need more data to continue. */
    PARSE_INCOMPLETE,
    /*! for responses that don't have length specified. */
    PARSE_INCOMPLETE_ENTITY,
    /*! parse failed; check status code for details. */
    PARSE_FAILURE,
    /*! done partial. */
    PARSE_OK,
    /*! token not matched. */
    PARSE_NO_MATCH,
    /*! private. */
    PARSE_CONTINUE_1
};

/// Structure of an HTTP header object.
struct http_header_t {
    /*! \brief Header name as a string. */
    memptr name;
    /*! \brief Header name id (for a selective group of headers only). */
    int name_id;
    /*! \brief Raw-value; could be multi-lined; min-length = 0. */
    membuffer value;
    /*! \brief (Private use -- don't touch.) */
    membuffer name_buf;
};

/// \brief Structure of an HTTP message.
struct http_message_t {
    /// \brief Indicates if the object is initialized.
    int initialized;

    /*! \name Used only for outgoing requests.
     * @{ */
    /// \brief Http method of an outgoing request.
    http_method_t method;
    /// \brief Type of a uri, e.g. absolute, relative, etc.
    uri_type uri;
    /// @}

    /*! \name Used only for incoming responses.
     * @{ */
    /// \brief Http method of an incoming response.
    http_method_t request_method;
    /// \brief ???
    int status_code;
    /// \brief ???
    membuffer status_msg;
    /*! \brief The amount of data that's been read by the user,
     * that's no longer in the raw message buffer. */
    size_t amount_discarded;
    /// @}

    /*! \name Used for outgoing requests or incoming responses.
     * @{ */
    /// \brief If 1, msg is a request, else response.
    int is_request;
    /// \brief Http major version.
    int major_version;
    /// \brief Http minor version.
    int minor_version;
    /// \brief List of headers.
    LinkedList headers;
    /// \brief message body(entity).
    memptr entity;
    /// @}

    /*! \name Private fields -- don't touch.
     * @{ */
    /// \brief entire raw message.
    membuffer msg;
    /// \brief storage for url string.
    char* urlbuf;
    /// @}
};

/// \brief Structure of an HTTP parser object.
struct http_parser_t {
    /*! \brief entire raw message
     * \details This contains the complete HTTP message with preceding header
     * and message body. There is also the http_parser_t::entity_start_position
     * available that points to the message body in the message. */
    http_message_t msg;
    /*! \brief read-only; in case of parse error, this
     * contains the HTTP error code (4XX or 5XX). */
    int http_error_code;
    /*! \brief read-only; this is set to 1 if a NOTIFY request has no
     * content-length. used to read valid sspd notify msg. */
    int valid_ssdp_notify_hack;
    /// @{
    /// \brief Private data -- don't touch.
    parser_pos_t position;
    int ent_position;
    unsigned int content_length;
    size_t chunk_size;
    /// @}
    /*! \brief Offset in the raw message buffer, which contains the message
     * body. preceding this are the headers of the message. */
    size_t entity_start_position;
    /// ???
    scanner_t scanner;
};

/*!
 * \brief Free memory allocated for the http message.
 */
UPNPLIB_API void httpmsg_destroy( //
    http_message_t* msg           ///< [in,out] HTTP Message Object.
);

/*!
 * \brief Compares the header name with the header names stored in the linked
 * list of messages.
 *
 * \returns
 *  - Pointer to a header on success
 *  - nullptr on failure
 */
UPNPLIB_API http_header_t* httpmsg_find_hdr_str(
    http_message_t* msg,    ///< [in] HTTP Message Object.
    const char* header_name ///< [in] Header name to be compared with.
);

/*!
 * \brief Finds header from a list, with the given 'name_id'.
 *
 * \returns
 *  - Pointer to a header on success
 *  - nullptr on failure
 */
UPNPLIB_API http_header_t* httpmsg_find_hdr(
    http_message_t* msg, ///< [in] HTTP Message Object.
    int header_name_id,  ///< [in] Header Name ID to be compared with.
    memptr* value        ///< [out] Buffer to get the ouput to.
);

/*!
 * \brief Initializes parser object for a request.
 */
UPNPLIB_API void parser_request_init( //
    http_parser_t* parser             ///< [out] HTTP Parser Object.
);

/*!
 * \brief Initializes parser object for a response.
 */
UPNPLIB_API void parser_response_init( //
    http_parser_t* parser,             ///< [out] HTTP Parser object.
    http_method_t request_method       ///< [in] Request method.
);

/*!
 * \brief The parser function.
 *
 * Depending on the position of the parser object the actual parsing function is
 * invoked.
 *
 *  \returns
 *  On success: PARSE_SUCCESS\n
 *  On error:
 *  - PARSE_FAILURE
 *  - PARSE_INCOMPLETE
 *  - PARSE_INCOMPLETE_ENTITY
 *  - PARSE_NO_MATCH
 */
parse_status_t parser_parse( //
    http_parser_t* parser    ///< [in,out] HTTP Parser Object.
);

/* **********************************************************************
 * Function: matchstr
 *
 * Parameters:
 *  IN char *str ;      String to be matched
 *  IN size_t slen ;    Length of the string
 *  IN const char* fmt ;    Pattern format
 *  ...
 *
 * Description: Matches a variable parameter list with a string
 *  and takes actions based on the data type specified.
 *
 * Returns:
 *   PARSE_OK
 *   PARSE_NO_MATCH -- failure to match pattern 'fmt'
 *   PARSE_FAILURE  -- 'str' is bad input
 *   PARSE_INCOMPLETE
 ************************************************************************/
/*!
 * \brief Get HTTP Method, URL location and version information.
 *
 * \returns
 *  - PARSE_OK
 *  - PARSE_SUCCESS
 *  - PARSE_FAILURE
 *
 *  \todo Check what function description is the right one. There was another
 * found.
 */
UPNPLIB_API parse_status_t parser_parse_responseline(
    http_parser_t* parser ///< [in,out] HTTP Parser object.
);

/* **********************************************************************
 * Function: parser_parse_headers
 *
 * Parameters:
 *  INOUT http_parser_t* parser ; HTTP Parser object
 *
 * Description: Read HTTP header fields.
 *
 * Returns:
 *  PARSE_OK
 *  PARSE_SUCCESS
 *  PARSE_FAILURE
 *  PARSE_INCOMPLETE
 *  PARSE_NO_MATCH
 ************************************************************************/
/*!
 * \brief Get HTTP Method, URL location and version information.
 *
 * \returns
 *  - PARSE_OK
 *  - PARSE_SUCCESS
 *  - PARSE_FAILURE
 *
 *  \todo Check what function description is the right one. There was another
 * found.
 */
UPNPLIB_API parse_status_t parser_parse_headers(
    http_parser_t* parser ///< [in,out] HTTP Parser object.
);

/* !
 * \brief Read HTTP entity body.
 *
 * \returns
 *  On success: PARSE_SUCCESS - no more reading to do.\n
 *  On error:
 *  - PARSE_FAILURE
 *  - PARSE_NO_MATCH
 *  - PARSE_INCOMPLETE
 *  - PARSE_INCOMPLETE_ENTITY
 */
/*!
 * \brief Determines method to read entity.
 *
 * \returns
 *  - PARSE_OK
 *  - PARSE_FAILURE
 *  - PARSE_COMPLETE -- no more reading to do
 *
 *  \todo Check what function description is the right one. There was another
 * found.
 */
UPNPLIB_API parse_status_t parser_parse_entity(
    http_parser_t* parser ///< [in,out] HTTP Parser object.
);

/*!
 * \brief Determines method to read entity.
 *
 * \returns
 *  On success: PARSE_CONTINUE_1\n
 *  On error:
 *  - PARSE_FAILURE
 *  - PARSE_SUCCESS  -- no more reading to do
 */
UPNPLIB_API parse_status_t parser_get_entity_read_method(
    http_parser_t* parser ///< [in,out] HTTP Parser object.
);

/*!
 * \brief Append date to HTTP parser, and do the parsing.
 *
 * \returns
 *  On success: PARSE_SUCCESS\n
 *  On error:
 *  - PARSE_FAILURE
 *  - PARSE_INCOMPLETE
 *  - PARSE_INCOMPLETE_ENTITY
 *  - PARSE_NO_MATCH
 */
UPNPLIB_API parse_status_t parser_append(
    http_parser_t* parser, ///< [in,out] HTTP Parser object.
    const char* buf,       ///< [in] buffer to be appended to the parser.
    size_t buf_length      ///< [in] Size of the buffer.
);

/*!
 * \brief Matches a variable parameter list with a string and takes actions
 * based on the data type specified.
 *
 * \returns
 *  - PARSE_OK
 *  - PARSE_NO_MATCH -- failure to match pattern 'fmt'
 *  - PARSE_FAILURE  -- 'str' is bad input
 */
parse_status_t matchstr( //
    char* str,           ///< [in] String to be matched.
    size_t slen,         ///< [in] Length of the string.
    const char* fmt,     ///< [in] Pattern format for arguments (like printf()).
    ...                  ///< [in] Variable list of arguments (like printf()).
);

/*!
 * \brief Converts raw character data to integer value.
 *
 * \returns integer
 */
int raw_to_int(        //
    memptr* raw_value, ///< [in] Buffer to be converted.
    int base           ///< Base to use for conversion.
);

/*!
 * \brief Find a substring from raw character string buffer.
 *
 * Side effects: raw_value is transformed to lowercase.
 *
 * \returns integer - index at which the substring is found.
 */
int raw_find_str(      //
    memptr* raw_value, ///< [in] Buffer containg the string.
    const char* str    ///< [in] Substring to be found.
);

/*!
 * \brief A wrapper function that maps a method id to a method.
 *
 * nameConverts a http_method id stored in the HTTP Method.
 *
 * \returns Pointer to the HTTP Method.
 */
UPNPLIB_API const char* method_to_str( //
    http_method_t method               ///< [in] HTTP method.
);

/*!
 * \brief Print the HTTP headers if DEBUG is enabled.
 * \details Function is only available when compiled with DEBUG enabled.
 */
#if defined(DEBUG) || defined(DOXYGEN_RUN)
UPNPLIB_API void print_http_headers( //
    http_message_t* hmsg             ///< [in] HTTP Message object.
);
#else
#define print_http_headers(hmsg)                                               \
    do {                                                                       \
    } while (0)
#endif

#endif /* COMPA_GENLIB_NET_HTTP_HTTPPARSER_HPP */
