/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-07-30
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
// Last compare with ./pupnp source file on 2023-08-03, ver 1.14.17

/*!
 * \file
 * \brief This file defines the functionality making use of the http.
 *
 * It defines functions to receive messages, process messages, send messages.
 */

#include <httpreadwrite.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>
#include <upnplib/sockaddr.hpp>

#include <UpnpExtraHeaders.hpp>
#include <UpnpIntTypes.hpp>
#include <statcodes.hpp>
#include <upnpapi.hpp>
#include <webserver.hpp>

/// \cond
#include <cassert>
#include <cstdarg>
#include <cstring>
/// \endcond

#ifdef _WIN32
/// \cond
#include <malloc.h>
/// \endcond
#define fseeko fseek
#else /* _WIN32 */
/// \cond
#include <sys/utsname.h>
/// \endcond
#endif /* _WIN32 */

#include <umock/pupnp_sock.hpp>
#include <umock/pupnp_httprw.hpp>
#include <umock/stdio.hpp>
#include <umock/sys_socket.hpp>
#include <umock/winsock2.hpp>
#include <umock/sysinfo.hpp>


namespace {

/// ???
constexpr size_t CHUNK_HEADER_SIZE{10};
/// ???
constexpr size_t CHUNK_TAIL_SIZE{10};

/* in seconds */
/*! \brief Default TCP connection timeout.
 */
constexpr time_t DEFAULT_TCP_CONNECT_TIMEOUT{5};


/*! \name Scope restricted to file
 * @{
 */
/*!
 * \brief Checks socket connection and wait if it is not connected.
 *
 * It should be called just after connect.
 *
 * \return 0 if successful, else -1.
 */
int Check_Connect_And_Wait_Connection(
    const SOCKET a_sock,  ///< [in] Socket file descriptor.
    const int connect_res /*!< [in] Result of connect that has been executed
                                    before calling this. */
) {
    TRACE("Executing Check_Connect_And_Wait_Connection()")

    if (connect_res == 0)
        return 0;
#ifdef _WIN32
    if (umock::winsock2_h.WSAGetLastError() != WSAEWOULDBLOCK)
        return -1;
#else
    if (errno != EINPROGRESS)
        return -1;
#endif

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(a_sock, &fdSet);
    timeval tmvTimeout = {DEFAULT_TCP_CONNECT_TIMEOUT, 0};
    int result{SOCKET_ERROR};

    result =
        umock::sys_socket_h.select(a_sock + 1, NULL, &fdSet, NULL, &tmvTimeout);
    switch (result) {
    case SOCKET_ERROR:
        return -1;
    case 0:
        /* timeout */
        return -1;
    }
    int valopt{};
    socklen_t len{sizeof(valopt)};
    if (umock::sys_socket_h.getsockopt(a_sock, SOL_SOCKET, SO_ERROR,
                                       (void*)&valopt, &len) < 0)
        /* failed to read delayed error */
        return -1;
    if (valopt)
        /* delayed error = valopt */
        // TODO: Return more detailed error codes, e.g. valopt == 111:
        // ECONNREFUSED "Connection refused" if there is a remote host but no
        // server service listening.
        return -1;

    return 0;
}

/// \cond
// Using this variable to be able to set it by unit tests to test
// blocking vs. unblocking at runtime, no need to compile it.
#ifdef COMPA_ENABLE_BLOCKING_TCP_CONNECTIONS
bool unblock_tcp_connections{false};
#else
bool unblock_tcp_connections{true};
#endif
/// \endcond

/*!
 * \brief Point to the hostname within a URL C-string.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error: UPNP_E_INVALID_URL
 *
 * \todo Check if this can be taken from the httpparser.
 */
int get_hoststr(
    const char* url_str, ///< [in] Pointer to URL string.
    const char**
        hoststr,    /*!< [out] Filled with a pointer to the start of the host
                       string within url_str. On error this remains untouched. */
    size_t* hostlen /*!< [out] Filled with the length (without terminating '\0')
                       of the host string. On error this remains untouched. */
) {
    TRACE("Executing get_hoststr()")
    const char* start;
    const char* finish;
    int ret_code = UPNP_E_INVALID_URL;

    // strlen() must be at least 2 char otherwise compiler complains boundary
    // error with next strstr() statement.
    if (strlen(url_str) < 2)
        goto end_function;

    start = strstr(url_str, "//");
    if (!start)
        goto end_function;

    start += 2;
    finish = strchr(start, '/');
    if (finish) {
        *hostlen = static_cast<size_t>(finish - start);
    } else {
        *hostlen = strlen(start);
    }
    *hoststr = start;

    ret_code = UPNP_E_SUCCESS;

end_function:
    return ret_code;
}

/*!
 * \brief Dummy function. It do nothing since years. Do we need it?
 *
 * \todo Dummy function. It do nothing since years. Do we need it?
 */
void copy_msg_headers([[maybe_unused]] LinkedList* msgHeaders,
                      [[maybe_unused]] UpnpString* headers) {
    return;
/* TODO: */
#if 0
    ListNode *node;
    UpnpHttpHeader *header;
    http_header_t *msgHeader;
    if (headers) {
        ListInit(headers, NULL, (free_function) UpnpHttpHeader_delete);
        node = ListHead(msgHeaders);
        while(node) {
            msgHeader = (http_header_t*) node->item;
            header = UpnpHttpHeader_new();
            UpnpHttpHeader_strncpy_Name(
                header,
                msgHeader->name.buf,
                msgHeader->name.length);
            UpnpHttpHeader_strncpy_Value(
                header,
                msgHeader->value.buf,
                msgHeader->value.length);
            node = ListNext(msgHeaders, node);
        }
    }
#endif
}

/*!
 * \brief Make a generic message, what ever this mean.
 *
 * \todo Need to be documented.
 */
int MakeGenericMessage(http_method_t method, const char* url_str,
                       membuffer* request, uri_type* url, int contentLength,
                       const char* contentType, const UpnpString* headers) {
    int ret_code = 0;
    size_t hostlen{};
    const char* hoststr;

    UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__, "URL: %s method: %d\n",
               url_str, method);
    ret_code = http_FixStrUrl(url_str, strlen(url_str), url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* make msg */
    membuffer_init(request);
    ret_code = http_MakeMessage(request, 1, 1, "Q", method, url->pathquery.buff,
                                url->pathquery.size);
    /* add request headers if specified, otherwise use default headers */
    if (ret_code == 0) {
        if (headers) {
            ret_code = http_MakeMessage(request, 1, 1, "s",
                                        UpnpString_get_String(headers));
        } else {
            ret_code = get_hoststr(url_str, &hoststr, &hostlen);
            if (ret_code != UPNP_E_SUCCESS)
                return ret_code;
            UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                       "HOSTNAME : %s Length : %" PRIzu "\n", hoststr, hostlen);
            ret_code = http_MakeMessage(request, 1, 1,
                                        "s"
                                        "bcDCU",
                                        "HOST: ", hoststr, hostlen);
        }
    }

    /* add the content-type header */
    if (ret_code == 0 && contentType) {
        ret_code = http_MakeMessage(request, 1, 1, "T", contentType);
    }
    /* add content-length header. */
    if (ret_code == 0) {
        if (contentLength >= 0)
            ret_code =
                http_MakeMessage(request, 1, 1, "Nc", (off_t)contentLength);
        else if (contentLength == UPNP_USING_CHUNKED)
            ret_code = http_MakeMessage(request, 1, 1, "Kc");
        else if (contentLength == UPNP_UNTIL_CLOSE)
            ret_code = http_MakeMessage(request, 1, 1, "c");
        else
            ret_code = UPNP_E_INVALID_PARAM;
    }
    if (ret_code != 0) {
        UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                   "HTTP Makemessage failed\n");
        membuffer_destroy(request);
        return ret_code;
    }
    UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
               "HTTP Buffer:\n%s\n"
               "----------END--------\n",
               request->buf);

    return UPNP_E_SUCCESS;
}

/*!
 * \brief Handle for a connection.
 */
struct http_connection_handle_t {
    SOCKINFO sock_info;
    int contentLength;
    http_parser_t response;
    int requestStarted;
    int cancel;
};

/*!
 * \brief Parses already exiting data.
 *
 * If not complete reads more data on the connected socket. The read data is
 * then parsed. The same methid is carried out for headers.
 *
 * \returns
 *  On success: PARSE_OK\n
 *  On error:
 *  - PARSE_FAILURE - Failure to parse data correctly
 *  - UPNP_E_BAD_HTTPMSG - Socker read() returns an error
 */
int ReadResponseLineAndHeaders(
    /*! Socket information object. */
    SOCKINFO* info,
    /*! HTTP Parser object. */
    http_parser_t* parser,
    /*! Time out value. */
    int* timeout_secs,
    /*! HTTP errror code returned. */
    int* http_error_code) {
    parse_status_t status;
    int num_read;
    char buf[2 * 1024];
    int done = 0;
    int ret_code = 0;

    /*read response line */
    status = parser_parse_responseline(parser);
    switch (status) {
    case PARSE_OK:
        done = 1;
        break;
    case PARSE_INCOMPLETE:
        done = 0;
        break;
    default:
        /*error */
        return status;
    }
    while (!done) {
        num_read = sock_read(info, buf, sizeof(buf), timeout_secs);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code =
                membuffer_append(&parser->msg.msg, buf, (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                parser->http_error_code = HTTP_INTERNAL_SERVER_ERROR;
                return PARSE_FAILURE;
            }
            status = parser_parse_responseline(parser);
            switch (status) {
            case PARSE_OK:
                done = 1;
                break;
            case PARSE_INCOMPLETE:
                done = 0;
                break;
            default:
                /*error */
                return status;
            }
        } else if (num_read == 0) {
            /* partial msg */
            *http_error_code = HTTP_BAD_REQUEST; /* or response */
            return UPNP_E_BAD_HTTPMSG;
        } else {
            *http_error_code = parser->http_error_code;
            return num_read;
        }
    }
    status = parser_parse_headers(parser);
    if ((status == (parse_status_t)PARSE_OK) &&
        (parser->position == (parser_pos_t)POS_ENTITY))
        done = 1;
    else if (status == (parse_status_t)PARSE_INCOMPLETE)
        done = 0;
    else
        /*error */
        return status;
    /*read headers */
    while (!done) {
        num_read = sock_read(info, buf, sizeof(buf), timeout_secs);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code =
                membuffer_append(&parser->msg.msg, buf, (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                parser->http_error_code = HTTP_INTERNAL_SERVER_ERROR;
                return PARSE_FAILURE;
            }
            status = parser_parse_headers(parser);
            if (status == (parse_status_t)PARSE_OK &&
                parser->position == (parser_pos_t)POS_ENTITY)
                done = 1;
            else if (status == (parse_status_t)PARSE_INCOMPLETE)
                done = 0;
            else
                /*error */
                return status;
        } else if (num_read == 0) {
            /* partial msg */
            *http_error_code = HTTP_BAD_REQUEST; /* or response */
            return UPNP_E_BAD_HTTPMSG;
        } else {
            *http_error_code = parser->http_error_code;
            return num_read;
        }
    }

    return PARSE_OK;
}

/*!
 * \brief Make extended GetMessage.
 * \todo Needs documentation. There was no information found.
 */
int MakeGetMessageEx(const char* url_str, membuffer* request, uri_type* url,
                     SendInstruction* pRangeSpecifier) {
    size_t url_str_len;
    int ret_code = UPNP_E_SUCCESS;
    size_t hostlen = 0;
    const char* hoststr;

    url_str_len = strlen(url_str);
    do {
        UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__, "DOWNLOAD URL : %s\n",
                   url_str);
        ret_code = http_FixStrUrl(url_str, url_str_len, url);
        if (ret_code != UPNP_E_SUCCESS) {
            break;
        }
        /* make msg */
        membuffer_init(request);
        ret_code = get_hoststr(url_str, &hoststr, &hostlen);
        if (ret_code != UPNP_E_SUCCESS) {
            break;
        }
        UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                   "HOSTNAME : %s Length : %" PRIzu "\n", hoststr, hostlen);
        ret_code = http_MakeMessage(request, 1, 1,
                                    "Q"
                                    "s"
                                    "bc"
                                    "GDCUc",
                                    HTTPMETHOD_GET, url->pathquery.buff,
                                    url->pathquery.size, "HOST: ", hoststr,
                                    hostlen, pRangeSpecifier);
        if (ret_code != 0) {
            UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                       "HTTP Makemessage failed\n");
            membuffer_destroy(request);
            return ret_code;
        }
    } while (0);
    UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
               "HTTP Buffer:\n%s\n"
               "----------END--------\n",
               request->buf);

    return ret_code;
}

/// @} // Functions (scope restricted to file)
} // anonymous namespace

/// @{
/*!
 * \brief Initiate a connection on a socket.
 *
 * \returns
 *  On success: 0\n
 *  On error: SOCKET_ERROR
 *
 * \todo Return more detailed error codes, e.g. with 'getsockopt()' valopt ==
 * 111: ECONNREFUSED "Connection refused" if there is a remote host but no
 * server service listening.
 */
// Due to compatibility with Umock for Pupnp this has to be defined static.
static int private_connect(
    const SOCKET sockfd, ///< [in] Socket file descriptor.
    const sockaddr* const
        serv_addr,          ///< [in] Socket address of a remote network node.
    const socklen_t addrlen ///< [in] Size of the socket address.
) {
    TRACE("Executing private_connect(), blocking " +
          std::string(unblock_tcp_connections ? "false" : "true"))

    if (unblock_tcp_connections) {
        // This is never used due to SDK specification. It is only presevered
        // for historical reasons.

        int ret{SOCKET_ERROR};
        // returns 0 if successful, else SOCKET_ERROR.
        ret = umock::pupnp_sock.sock_make_no_blocking(sockfd);
        if (ret == 0) {
            // ret is needed for Check_Connect_And_Wait_Connection(),
            // returns 0 if successful, else -1.
            ret = umock::sys_socket_h.connect(sockfd, serv_addr, addrlen);
            // returns 0 if successful, else -1.
            ret = Check_Connect_And_Wait_Connection(sockfd, ret);

            // Always make_blocking() to revert make_no_blocking() above.
            // returns 0 if successful, else SOCKET_ERROR.
            ret = ret | umock::pupnp_sock.sock_make_blocking(sockfd);
        }

        return ret == 0 ? 0 : SOCKET_ERROR;

    } else { // tcp_connection is blocking

        upnplib::CSocketErr serrObj;
        if (umock::sys_socket_h.connect(sockfd, serv_addr, addrlen) != 0) {
            serrObj.catch_error();
            UPNPLIB_LOGERR "MSG1020: failed to connect() socket("
                << sockfd << "): " << serrObj.error_str() << "\n";

            return SOCKET_ERROR;
        }
        return 0;
    }
}
/// @}

#if defined(_WIN32) || defined(DOXYGEN_RUN)
tm* http_gmtime_r(const time_t* clock, tm* result) {
    if (clock == NULL || *clock < 0 || result == NULL)
        return NULL;

    /* gmtime in VC runtime is thread safe. */
    gmtime_s(result, clock);
    return result;
}
#endif

int http_FixUrl(uri_type* url, uri_type* fixed_url) {
    const char* temp_path = "/";

    *fixed_url = *url;
#ifdef UPNP_ENABLE_OPEN_SSL
    if (token_string_casecmp(&fixed_url->scheme, "http") != 0 &&
        token_string_casecmp(&fixed_url->scheme, "https") != 0) {
        return UPNP_E_INVALID_URL;
    }
#else
    if (token_string_casecmp(&fixed_url->scheme, "http") != 0) {
        return UPNP_E_INVALID_URL;
    }
#endif
    if (fixed_url->hostport.text.size == (size_t)0) {
        return UPNP_E_INVALID_URL;
    }
    /* set pathquery to "/" if it is empty */
    if (fixed_url->pathquery.size == (size_t)0) {
        fixed_url->pathquery.buff = temp_path;
        fixed_url->pathquery.size = (size_t)1;
    }

    return UPNP_E_SUCCESS;
}

int http_FixStrUrl(const char* urlstr, size_t urlstrlen, uri_type* fixed_url) {
    uri_type url;

    if (parse_uri(urlstr, urlstrlen, &url) != HTTP_SUCCESS) {
        return UPNP_E_INVALID_URL;
    }

    return http_FixUrl(&url, fixed_url);
}

SOCKET http_Connect(uri_type* destination_url, uri_type* url) {
    SOCKET connfd;
    socklen_t sockaddr_len;
    int ret_connect;

    // BUG! Must check return value. --Ingo
    http_FixUrl(destination_url, url);

    connfd = umock::sys_socket_h.socket((int)url->hostport.IPaddress.ss_family,
                                        SOCK_STREAM, 0);
    if (connfd == INVALID_SOCKET) {
        return (SOCKET)(UPNP_E_OUTOF_SOCKET);
    }
    sockaddr_len = (socklen_t)(url->hostport.IPaddress.ss_family == AF_INET6
                                   ? sizeof(struct sockaddr_in6)
                                   : sizeof(struct sockaddr_in));
    ret_connect = umock::pupnp_httprw.private_connect(
        connfd, (struct sockaddr*)&url->hostport.IPaddress, sockaddr_len);
    if (ret_connect == -1) {
#ifdef _WIN32
        UpnpPrintf(UPNP_CRITICAL, HTTP, __FILE__, __LINE__,
                   "connect error: %d\n", umock::winsock2_h.WSAGetLastError());
#endif
        if (umock::sys_socket_h.shutdown(connfd, SD_BOTH) == -1) {
            UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                       "Error in shutdown: %s\n", std::strerror(errno));
        }
        umock::unistd_h.CLOSE_SOCKET_P(connfd);
        return (SOCKET)(UPNP_E_SOCKET_CONNECT);
    }

    return connfd;
}

int http_RecvMessage(SOCKINFO* info, http_parser_t* parser,
                     http_method_t request_method, int* timeout_secs,
                     int* http_error_code) {
    TRACE("Executing http_RecvMessage()")
    int ret{UPNP_E_SUCCESS};
    int line{};
    parse_status_t status{};
    int num_read{};
    int ok_on_close{};
    char* buf{nullptr};
    size_t buf_len{1024};

    *http_error_code = HTTP_INTERNAL_SERVER_ERROR;
    buf = (char*)malloc(buf_len);
    if (!buf) {
        ret = UPNP_E_OUTOF_MEMORY;
        goto ExitFunction;
    }
    if (request_method == (http_method_t)HTTPMETHOD_UNKNOWN) {
        parser_request_init(parser);
    } else {
        parser_response_init(parser, request_method);
    }

    while (1) {
        /* Double the bet if needed */
        /* We should have already exited the loop if the value was <= 0
         * so the cast is safe */
        if ((size_t)num_read >= buf_len) {
            free(buf);
            buf_len = 2 * buf_len;
            buf = (char*)malloc(buf_len);
            if (!buf) {
                ret = UPNP_E_OUTOF_MEMORY;
                goto ExitFunction;
            }
        }
        num_read = sock_read(info, buf, buf_len, timeout_secs);
        if (num_read > 0) {
            /* got data */
            status = parser_append(parser, buf, (size_t)num_read);
            switch (status) {
            case PARSE_SUCCESS:
                UPNPLIB_LOGINFO "MSG1031: <<< (RECVD) <<<\n"
                    << parser->msg.msg.buf << "UPnPlib -----------------\n";
                print_http_headers(&parser->msg);
                if (g_maxContentLength > (size_t)0 &&
                    parser->content_length > (unsigned int)g_maxContentLength) {
                    *http_error_code = HTTP_REQ_ENTITY_TOO_LARGE;
                    line = __LINE__;
                    ret = UPNP_E_OUTOF_BOUNDS;
                    goto ExitFunction;
                }
                line = __LINE__;
                ret = 0;
                goto ExitFunction;
            case PARSE_FAILURE:
            case PARSE_NO_MATCH:
                *http_error_code = parser->http_error_code;
                line = __LINE__;
                ret = UPNP_E_BAD_HTTPMSG;
                goto ExitFunction;
            case PARSE_INCOMPLETE_ENTITY:
                /* read until close */
                ok_on_close = 1;
                break;
            case PARSE_CONTINUE_1:
                /* Web post request. */
                line = __LINE__;
                ret = PARSE_SUCCESS;
                goto ExitFunction;
            default:
                break;
            }
        } else if (num_read == 0) {
            if (ok_on_close) {
                UPNPLIB_LOGINFO "MSG1047: <<< (RECVD) <<<\n"
                    << parser->msg.msg.buf << "\n-----------------\n";
                print_http_headers(&parser->msg);
                line = __LINE__;
                ret = UPNP_E_SUCCESS;
                goto ExitFunction;
            } else {
                /* partial msg */
                *http_error_code = HTTP_BAD_REQUEST; /* or response */
                line = __LINE__;
                ret = UPNP_E_BAD_HTTPMSG;
                goto ExitFunction;
            }
        } else {
            *http_error_code = parser->http_error_code;
            line = __LINE__;
            ret = num_read;
            goto ExitFunction;
        }
    }

ExitFunction:
    free(buf);
    if (ret != UPNP_E_SUCCESS) {
        UPNPLIB_LOGERR << "MSG1048: " << ret << " on line " << line
                       << ", http_error_code = " << *http_error_code << ".\n";
    }

    return ret;
}

int http_SendMessage(SOCKINFO* info, int* TimeOut, const char* fmt, ...) {
    TRACE("Executing http_SendMessage()")
#ifdef COMPA_HAVE_WEBSERVER
    FILE* Fp{nullptr};
    SendInstruction* Instr{nullptr};
    char* filename{nullptr};
    char* file_buf{nullptr};
    char* ChunkBuf{nullptr};
    /* 10 byte allocated for chunk header. */
    char Chunk_Header[CHUNK_HEADER_SIZE];
    size_t num_read;
    off_t amount_to_be_read{};
    size_t Data_Buf_Size{WEB_SERVER_BUF_SIZE};
#endif /* COMPA_HAVE_WEBSERVER */
    va_list argp;
    char* buf{nullptr};
    char c;
    int nw;
    int RetVal = UPNP_E_SUCCESS;
    size_t buf_length;
    size_t num_written;

#ifdef COMPA_HAVE_WEBSERVER
    int I_fmt_processed = 0;
    memset(Chunk_Header, 0, sizeof(Chunk_Header));
#endif
    va_start(argp, fmt);
    while ((c = *fmt++) != '\0') {
#ifdef COMPA_HAVE_WEBSERVER
        if (c == 'I' && !I_fmt_processed) {
            I_fmt_processed = 1;
            Instr = va_arg(argp, struct SendInstruction*);
            if (Instr->ReadSendSize >= 0) {
                amount_to_be_read = Instr->ReadSendSize;
            } else {
                amount_to_be_read = (off_t)Data_Buf_Size;
            }
            if (amount_to_be_read < (off_t)WEB_SERVER_BUF_SIZE)
                Data_Buf_Size = (size_t)amount_to_be_read;
            ChunkBuf = (char*)malloc(
                (size_t)(Data_Buf_Size + CHUNK_HEADER_SIZE + CHUNK_TAIL_SIZE));
            if (!ChunkBuf) {
                RetVal = UPNP_E_OUTOF_MEMORY;
                goto ExitFunction;
            }
            file_buf = ChunkBuf + CHUNK_HEADER_SIZE;
        } else if (c == 'f') {
            /* file name */
            filename = va_arg(argp, char*);
            if (Instr && Instr->IsVirtualFile)
                Fp = (FILE*)(virtualDirCallback.open)(
                    filename, UPNP_READ, Instr->Cookie, Instr->RequestCookie);
            else
#ifdef _WIN32
                // returns error code direct
                if (umock::stdio_h.fopen_s(&Fp, filename, "rb") != 0)
                    Fp = nullptr;
#else
                Fp = umock::stdio_h.fopen(filename, "rb");
#endif
            if (Fp == nullptr) {
                RetVal = UPNP_E_FILE_READ_ERROR;
                goto ExitFunction;
            }
            if (Instr && Instr->IsRangeActive && Instr->IsVirtualFile) {
                if (virtualDirCallback.seek(Fp, Instr->RangeOffset, SEEK_CUR,
                                            Instr->Cookie,
                                            Instr->RequestCookie) != 0) {
                    RetVal = UPNP_E_FILE_READ_ERROR;
                    goto Cleanup_File;
                }
            } else if (Instr && Instr->IsRangeActive) {
                if (fseeko(Fp, Instr->RangeOffset, SEEK_CUR) != 0) {
                    RetVal = UPNP_E_FILE_READ_ERROR;
                    goto Cleanup_File;
                }
            }
            while (amount_to_be_read) {
                if (Instr) {
                    int nr;
                    size_t n = amount_to_be_read >= (off_t)Data_Buf_Size
                                   ? Data_Buf_Size
                                   : (size_t)amount_to_be_read;
                    if (Instr->IsVirtualFile) {
                        nr = virtualDirCallback.read(Fp, file_buf, n,
                                                     Instr->Cookie,
                                                     Instr->RequestCookie);
                        num_read = (size_t)nr;
                    } else {
                        num_read =
                            umock::stdio_h.fread(file_buf, (size_t)1, n, Fp);
                        TRACE("Read " + std::to_string(num_read) +
                              " bytes from input file = \"" +
                              std::string(file_buf) + "\"")
                    }
                    amount_to_be_read -= (off_t)num_read;
                    if (Instr->ReadSendSize < 0) {
                        /* read until close */
                        amount_to_be_read = (off_t)Data_Buf_Size;
                    }
                } else {
                    num_read = umock::stdio_h.fread(file_buf, (size_t)1,
                                                    Data_Buf_Size, Fp);
                }
                if (num_read == (size_t)0) {
                    /* EOF so no more to send. */
                    if (Instr && Instr->IsChunkActive) {
                        const char* str = "0\r\n\r\n";
                        nw = sock_write(info, str, strlen(str), TimeOut);
                    } else {
                        RetVal = UPNP_E_FILE_READ_ERROR;
                    }
                    goto Cleanup_File;
                }
                /* Create chunk for the current buffer. */
                if (Instr && Instr->IsChunkActive) {
                    int rc;
                    /* Copy CRLF at the end of the chunk */
                    memcpy(file_buf + num_read, "\r\n", (size_t)2);
                    /* Hex length for the chunk size. */
                    memset(Chunk_Header, 0, sizeof(Chunk_Header));
                    rc = snprintf(Chunk_Header, sizeof(Chunk_Header),
                                  "%" PRIzx "\r\n", (unsigned long)num_read);
                    if (rc < 0 || (unsigned int)rc >= sizeof(Chunk_Header)) {
                        RetVal = UPNP_E_INTERNAL_ERROR;
                        goto Cleanup_File;
                    }
                    /* Copy the chunk size header  */
                    memcpy(file_buf - strlen(Chunk_Header), Chunk_Header,
                           strlen(Chunk_Header));
                    /* on the top of the buffer. */
                    /*file_buf[num_read+strlen(Chunk_Header)]
                     * = NULL; */
                    /*upnpprintf("Sending
                     * %s\n",file_buf-strlen(Chunk_Header));*/
                    nw = sock_write(info, file_buf - strlen(Chunk_Header),
                                    num_read + strlen(Chunk_Header) + (size_t)2,
                                    TimeOut);
                    num_written = static_cast<size_t>(nw);
                    if (nw <= 0 || num_written != num_read +
                                                      strlen(Chunk_Header) +
                                                      (size_t)2)
                        /* Send error nothing we can do.
                         */
                        goto Cleanup_File;
                } else {
                    /* write data */
                    nw = sock_write(info, file_buf, num_read, TimeOut);
                    UPNPLIB_LOGINFO "MSG1104: >>> (SENT) >>>\n"
                        << std::string(file_buf, static_cast<size_t>(nw))
                        << "UPnPlib ------------\n";
                    /* Send error nothing we can do */
                    num_written = (size_t)nw;
                    if (nw <= 0 || num_written != num_read) {
                        goto Cleanup_File;
                    }
                }
            } /* while */
        Cleanup_File:
            if (Instr && Instr->IsVirtualFile) {
                virtualDirCallback.close(Fp, Instr->Cookie,
                                         Instr->RequestCookie);
            } else {
                umock::stdio_h.fclose(Fp);
            }
            goto ExitFunction;
        } else
#endif /* COMPA_HAVE_WEBSERVER */
            if (c == 'b') {
                /* Message to send is given in a memory buffer */
                buf = va_arg(argp, char*);
                buf_length = va_arg(argp, size_t);
                if (buf_length > (size_t)0) {
                    nw = sock_write(info, buf, buf_length, TimeOut);
                    num_written = (size_t)nw;

                    if (upnplib::g_dbug) {
                        upnplib::SSockaddr saObj;
                        memcpy(&saObj.ss, &info->foreign_sockaddr,
                               saObj.sizeof_ss());
                        UPNPLIB_LOGINFO "MSG1105: >>> (SENT) >>> to \""
                            << saObj.netaddrp() << "\"\n"
                            << std::string(buf, buf_length)
                            << "UPnPlib buf_length=" << buf_length
                            << ", num_written=" << num_written
                            << ".\nUPnPlib ------------\n";
                    }
                    if (nw < 0) {
                        RetVal = nw;
                        goto ExitFunction;
                    }
                    if (num_written != buf_length) {
                        RetVal = UPNP_E_SOCKET_WRITE;
                        goto ExitFunction;
                    }
                }
            }
    }

ExitFunction:
    va_end(argp);
#ifdef COMPA_HAVE_WEBSERVER
    free(ChunkBuf);
#endif
    return RetVal;
}


int http_RequestAndResponse(uri_type* destination, const char* request,
                            size_t request_length, http_method_t req_method,
                            int timeout_secs, http_parser_t* response) {
    TRACE("Executing http_RequestAndResponse()")
    SOCKET tcp_connection;
    int ret_code;
    socklen_t sockaddr_len;
    int http_error_code;
    SOCKINFO info;

    tcp_connection = umock::sys_socket_h.socket(
        (int)destination->hostport.IPaddress.ss_family, SOCK_STREAM, 0);
    if (tcp_connection == INVALID_SOCKET) {
        parser_response_init(response, req_method);
        return UPNP_E_SOCKET_ERROR;
    }
    if (sock_init(&info, tcp_connection) != UPNP_E_SUCCESS) {
        parser_response_init(response, req_method);
        ret_code = UPNP_E_SOCKET_ERROR;
        goto end_function;
    }
    /* connect */
    sockaddr_len = destination->hostport.IPaddress.ss_family == AF_INET6
                       ? sizeof(sockaddr_in6)
                       : sizeof(sockaddr_in);
    ret_code = umock::pupnp_httprw.private_connect(
        info.socket, (sockaddr*)&(destination->hostport.IPaddress),
        sockaddr_len);
    if (ret_code == -1) {
        parser_response_init(response, req_method);
        ret_code = UPNP_E_SOCKET_CONNECT;
        goto end_function;
    }
    /* send request */
    ret_code =
        http_SendMessage(&info, &timeout_secs, "b", request, request_length);
    if (ret_code != 0) {
        parser_response_init(response, req_method);
        goto end_function;
    }
    /* recv response */
    ret_code = http_RecvMessage(&info, response, req_method, &timeout_secs,
                                &http_error_code);

end_function:
    /* should shutdown completely */
    sock_destroy(&info, SD_BOTH);

    return ret_code;
}


int http_Download(const char* url_str, int timeout_secs, char** document,
                  size_t* doc_length, char* content_type) {
    TRACE("Executing http_Download()")
    int ret_code;
    uri_type url;
    char* msg_start;
    char* entity_start;
    const char* hoststr;
    size_t hostlen;
    http_parser_t response;
    size_t msg_length;
    memptr ctype;
    size_t copy_len;
    membuffer request;
    size_t url_str_len;

    url_str_len = strlen(url_str);
    /*ret_code = parse_uri( (char*)url_str, url_str_len, &url ); */
    UPNPLIB_LOGINFO "MSG1098: DOWNLOAD URL=\"" << url_str << "\".\n";
    ret_code = http_FixStrUrl((char*)url_str, url_str_len, &url);
    if (ret_code != UPNP_E_SUCCESS) {
        return ret_code;
    }
    /* make msg */
    membuffer_init(&request);
    ret_code = get_hoststr(url_str, &hoststr, &hostlen);
    if (ret_code != UPNP_E_SUCCESS) {
        return ret_code;
    }
    UPNPLIB_LOGINFO "MSG1099: HOSTNAME=\"" << hoststr
                                           << "\", Length=" << hostlen << "\n";
    ret_code = http_MakeMessage(&request, 1, 1,
                                "Q"
                                "s"
                                "bcDCUc",
                                HTTPMETHOD_GET, url.pathquery.buff,
                                url.pathquery.size, "HOST: ", hoststr, hostlen);
    if (ret_code != 0) {
        UPNPLIB_LOGINFO "MSG1100: HTTP Makemessage failed.\n";
        membuffer_destroy(&request);
        return ret_code;
    }
    UPNPLIB_LOGINFO "MSG1101: HTTP Buffer...\n"
        << request.buf << "UPnPlib ----------END--------\n";
    /* get doc msg */
    ret_code = http_RequestAndResponse(&url, request.buf, request.length,
                                       HTTPMETHOD_GET, timeout_secs, &response);

    if (ret_code != 0) {
        httpmsg_destroy(&response.msg);
        membuffer_destroy(&request);
        return ret_code;
    }
    UPNPLIB_LOGINFO "MSG1102: Response...\n";
    print_http_headers(&response.msg);
    /* optional content-type */
    if (content_type) {
        if (httpmsg_find_hdr(&response.msg, HDR_CONTENT_TYPE, &ctype) == NULL) {
            *content_type = '\0'; /* no content-type */
        } else {
            /* safety */
            copy_len = ctype.length < LINE_SIZE - (size_t)1
                           ? ctype.length
                           : LINE_SIZE - (size_t)1;

            memcpy(content_type, ctype.buf, copy_len);
            content_type[copy_len] = '\0';
        }
    }
    /* extract doc from msg */
    if ((*doc_length = response.msg.entity.length) == (size_t)0) {
        /* 0-length msg */
        *document = NULL;
    } else if (response.msg.status_code == HTTP_OK) {
        /*LEAK_FIX_MK */
        /* copy entity */
        entity_start = response.msg.entity.buf; /* what we want */
        msg_length = response.msg.msg.length;   /* save for posterity   */
        msg_start = membuffer_detach(&response.msg.msg); /* whole msg */
        /* move entity to the start; copy null-terminator too */
        memmove(msg_start, entity_start, *doc_length + (size_t)1);
        /* save mem for body only */
        *document =
            (char*)realloc(msg_start, *doc_length + (size_t)1); /*LEAK_FIX_MK */
        /* *document = Realloc( msg_start,msg_length, *doc_length + 1 );
         * LEAK_FIX_MK */
        /* shrink can't fail */
        assert(msg_length > *doc_length);
        assert(*document != NULL);
        if (msg_length <= *doc_length || *document == NULL)
            UPNPLIB_LOGINFO "MSG1103: msg_length("
                << msg_length << ") <= *doc_length(" << *doc_length
                << ") or document is NULL.\n";
    }
    if (response.msg.status_code == HTTP_OK) {
        ret_code = 0; /* success */
    } else {
        /* server sent error msg (not requested doc) */
        ret_code = response.msg.status_code;
    }
    httpmsg_destroy(&response.msg);
    membuffer_destroy(&request);

    return ret_code;
}


int http_HttpGetProgress(void* Handle, size_t* length, size_t* total) {
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;

    if (!handle || !length || !total) {
        return UPNP_E_INVALID_PARAM;
    }
    *length = handle->response.msg.entity.length;
    *total = handle->response.content_length;

    return UPNP_E_SUCCESS;
}

int http_CancelHttpGet(void* Handle) {
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;

    if (!handle)
        return UPNP_E_INVALID_PARAM;
    handle->cancel = 1;

    return UPNP_E_SUCCESS;
}

int http_OpenHttpConnection(const char* url_str, void** Handle,
                            [[maybe_unused]] int timeout) {
    TRACE("Executing http_OpenHttpConnection()")
    int ret_code;
    size_t sockaddr_len;
    SOCKET tcp_connection;
    http_connection_handle_t* handle = nullptr;
    uri_type url;
    // BUG! *Handle should be initialized here? Otherwise possible segfault.
    // *Handle = handle;
    if (!url_str || !Handle)
        return UPNP_E_INVALID_PARAM;
    *Handle = handle;
    /* parse url_str */
    ret_code = http_FixStrUrl(url_str, strlen(url_str), &url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* create the connection handle */
    handle =
        (http_connection_handle_t*)malloc(sizeof(http_connection_handle_t));
    if (!handle) {
        return UPNP_E_OUTOF_MEMORY;
    }
    handle->requestStarted = 0;
    memset(&handle->response, 0, sizeof(handle->response));
    /* connect to the server */
    tcp_connection = umock::sys_socket_h.socket(
        url.hostport.IPaddress.ss_family, SOCK_STREAM, 0);
    if (tcp_connection == INVALID_SOCKET) {
        handle->sock_info.socket = INVALID_SOCKET;
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    if (sock_init(&handle->sock_info, tcp_connection) != UPNP_E_SUCCESS) {
        sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    sockaddr_len = url.hostport.IPaddress.ss_family == AF_INET6
                       ? sizeof(struct sockaddr_in6)
                       : sizeof(struct sockaddr_in);
    ret_code = umock::pupnp_httprw.private_connect(
        handle->sock_info.socket, (struct sockaddr*)&(url.hostport.IPaddress),
        (socklen_t)sockaddr_len);
    if (ret_code == -1) {
        sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_CONNECT;
        goto errorHandler;
    }
#ifdef UPNP_ENABLE_OPEN_SSL
    /* For HTTPS connections start the TLS/SSL handshake. */
    if (token_string_casecmp(&url.scheme, "https") == 0) {
        ret_code = sock_ssl_connect(&handle->sock_info);
        if (ret_code != UPNP_E_SUCCESS) {
            sock_destroy(&handle->sock_info, SD_BOTH);
            goto errorHandler;
        }
    }
#endif
errorHandler:
    *Handle = handle;
    return ret_code;
}

int http_MakeHttpRequest(Upnp_HttpMethod method, const char* url_str,
                         void* Handle, UpnpString* headers,
                         const char* contentType, int contentLength,
                         int timeout) {
    int ret_code;
    membuffer request;
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    uri_type url;
    if (!url_str || !Handle)
        return UPNP_E_INVALID_PARAM;
    if (handle->requestStarted) {
        /* TODO: Log an error that a previous request is already in
         * progress. */
    }
    handle->requestStarted = 1;
    handle->cancel = 0;
    ret_code = MakeGenericMessage((http_method_t)method, url_str, &request,
                                  &url, contentLength, contentType, headers);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* send request */
    ret_code = http_SendMessage(&handle->sock_info, &timeout, "b", request.buf,
                                request.length);
    membuffer_destroy(&request);
    httpmsg_destroy(&handle->response.msg);
    parser_response_init(&handle->response, (http_method_t)method);
    return ret_code;
}

int http_WriteHttpRequest(void* Handle, char* buf, size_t* size, int timeout) {
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    char* tempbuf = NULL;
    size_t tempbufSize = 0;
    int freeTempbuf = 0;
    int numWritten = 0;

    if (!handle || !size || !buf) {
        if (size)
            *size = 0;
        return UPNP_E_INVALID_PARAM;
    }
    if (handle->contentLength == UPNP_USING_CHUNKED) {
        if (*size) {
            size_t tempSize = 0;
            size_t tempbuf_len{*size + CHUNK_HEADER_SIZE + CHUNK_TAIL_SIZE};
            tempbuf = (char*)malloc(tempbuf_len);
            if (!tempbuf)
                return UPNP_E_OUTOF_MEMORY;
            /* begin chunk */
            snprintf(tempbuf, tempbuf_len, "%zx\r\n", *size);
            tempSize = strlen(tempbuf);
            memcpy(tempbuf + tempSize, buf, *size);
            memcpy(tempbuf + tempSize + *size, "\r\n", 2);
            /* end of chunk */
            tempbufSize = tempSize + *size + 2;
            freeTempbuf = 1;
        }
    } else {
        tempbuf = buf;
        tempbufSize = *size;
    }
    numWritten = sock_write(&handle->sock_info, tempbuf, tempbufSize, &timeout);
    if (freeTempbuf)
        free(tempbuf);
    if (numWritten < 0) {
        *size = 0;
        return numWritten;
    } else {
        *size = (size_t)numWritten;
        return UPNP_E_SUCCESS;
    }
}

int http_EndHttpRequest(void* Handle, int timeout) {
    int retc = 0;
    const char* zcrlf = "0\r\n\r\n";
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    if (!handle)
        return UPNP_E_INVALID_PARAM;
    if (!handle->requestStarted) {
        return UPNP_E_SUCCESS;
    }
    handle->requestStarted = 0;
    if (handle->contentLength == UPNP_USING_CHUNKED)
        /*send last chunk */
        retc = sock_write(&handle->sock_info, zcrlf, strlen(zcrlf), &timeout);

    return retc >= 0 ? UPNP_E_SUCCESS : UPNP_E_SOCKET_WRITE;
}

int http_GetHttpResponse(void* Handle, UpnpString* headers, char** contentType,
                         int* contentLength, int* httpStatus, int timeout) {
    int ret_code;
    int http_error_code;
    memptr ctype;
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    parse_status_t status;

    status = (parse_status_t)ReadResponseLineAndHeaders(
        &handle->sock_info, &handle->response, &timeout, &http_error_code);
    if (status != (parse_status_t)PARSE_OK) {
        ret_code = UPNP_E_BAD_RESPONSE;
        goto errorHandler;
    }
    status = parser_get_entity_read_method(&handle->response);
    switch (status) {
    case PARSE_CONTINUE_1:
    case PARSE_SUCCESS:
        break;
    default:
        ret_code = UPNP_E_BAD_RESPONSE;
        goto errorHandler;
    }
    ret_code = UPNP_E_SUCCESS;
    if (httpStatus) {
        *httpStatus = handle->response.msg.status_code;
    }
    if (contentType) {
        if (!httpmsg_find_hdr(&handle->response.msg, HDR_CONTENT_TYPE, &ctype))
            /* no content-type */
            *contentType = NULL;
        else
            *contentType = ctype.buf;
    }
    if (contentLength) {
        if (handle->response.position == (parser_pos_t)POS_COMPLETE)
            *contentLength = 0;
        else if (handle->response.ent_position == ENTREAD_USING_CHUNKED)
            *contentLength = UPNP_USING_CHUNKED;
        else if (handle->response.ent_position == ENTREAD_USING_CLEN)
            *contentLength = (int)handle->response.content_length;
        else if (handle->response.ent_position == ENTREAD_UNTIL_CLOSE)
            *contentLength = UPNP_UNTIL_CLOSE;
    }

    if (headers) {
        copy_msg_headers(&handle->response.msg.headers, headers);
    }

errorHandler:
    if (ret_code != UPNP_E_SUCCESS)
        httpmsg_destroy(&handle->response.msg);
    return ret_code;
}

int http_ReadHttpResponse(void* Handle, char* buf, size_t* size, int timeout) {
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    parse_status_t status;
    int num_read;
    int ok_on_close = 0;
    char tempbuf[2 * 1024];
    int ret_code = 0;

    if (!handle || !size || (*size > 0 && !buf)) {
        if (size)
            *size = 0;
        return UPNP_E_INVALID_PARAM;
    }
    /* first parse what has already been gotten */
    if (handle->response.position != POS_COMPLETE)
        status = parser_parse_entity(&handle->response);
    else
        status = PARSE_SUCCESS;
    if (status == PARSE_INCOMPLETE_ENTITY)
        /* read until close */
        ok_on_close = 1;
    else if ((status != PARSE_SUCCESS) && (status != PARSE_CONTINUE_1) &&
             (status != PARSE_INCOMPLETE)) {
        /*error */
        *size = 0;
        return UPNP_E_BAD_RESPONSE;
    }
    /* read more if necessary entity */
    while (handle->response.msg.amount_discarded + *size >
               handle->response.msg.entity.length &&
           !handle->cancel && handle->response.position != POS_COMPLETE) {
        num_read =
            sock_read(&handle->sock_info, tempbuf, sizeof(tempbuf), &timeout);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code = membuffer_append(&handle->response.msg.msg, tempbuf,
                                        (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                handle->response.http_error_code = HTTP_INTERNAL_SERVER_ERROR;
                *size = 0;
                return PARSE_FAILURE;
            }
            status = parser_parse_entity(&handle->response);
            if (status == PARSE_INCOMPLETE_ENTITY) {
                /* read until close */
                ok_on_close = 1;
            } else if ((status != PARSE_SUCCESS) &&
                       (status != PARSE_CONTINUE_1) &&
                       (status != PARSE_INCOMPLETE)) {
                /*error */
                *size = 0;
                return UPNP_E_BAD_RESPONSE;
            }
        } else if (num_read == 0) {
            if (ok_on_close) {
                UpnpPrintf(UPNP_INFO, HTTP, __FILE__, __LINE__,
                           "<<< (RECVD) "
                           "<<<\n%s\n-----------------\n",
                           handle->response.msg.msg.buf);
                handle->response.position = POS_COMPLETE;
            } else {
                /* partial msg */
                *size = 0;
                handle->response.http_error_code =
                    HTTP_BAD_REQUEST; /* or response */
                return UPNP_E_BAD_HTTPMSG;
            }
        } else {
            *size = 0;
            return num_read;
        }
    }
    if (handle->cancel) {
        return UPNP_E_CANCELED;
    }
    /* truncate size to fall within available data */
    if (handle->response.msg.amount_discarded + *size >
        handle->response.msg.entity.length)
        *size = handle->response.msg.entity.length -
                handle->response.msg.amount_discarded;
    /* copy data to user buffer. delete copied data */
    if (*size > 0) {
        memcpy(buf,
               &handle->response.msg.msg
                    .buf[handle->response.entity_start_position],
               *size);
        membuffer_delete(&handle->response.msg.msg,
                         handle->response.entity_start_position, *size);
        /* update scanner position. needed for chunked transfers */
        handle->response.scanner.cursor -= *size;
        /* update amount discarded */
        handle->response.msg.amount_discarded += *size;
    }

    return UPNP_E_SUCCESS;
}

int http_CloseHttpConnection(void* Handle) {
    http_connection_handle_t* handle = (http_connection_handle_t*)Handle;
    if (!handle)
        return UPNP_E_INVALID_PARAM;
    /*should shutdown completely */
    sock_destroy(&handle->sock_info, SD_BOTH);
    httpmsg_destroy(&handle->response.msg);
    free(handle);
    return UPNP_E_SUCCESS;
}

int http_SendStatusResponse(SOCKINFO* info, int http_status_code,
                            int request_major_version,
                            int request_minor_version) {
    int response_major, response_minor;
    membuffer membuf;
    int ret;
    int timeout;

    http_CalcResponseVersion(request_major_version, request_minor_version,
                             &response_major, &response_minor);
    membuffer_init(&membuf);
    membuf.size_inc = (size_t)70;
    /* response start line */
    ret = http_MakeMessage(&membuf, response_major, response_minor, "RSCB",
                           http_status_code, http_status_code);
    if (ret == 0) {
        timeout = HTTP_DEFAULT_TIMEOUT;
        ret = http_SendMessage(info, &timeout, "b", membuf.buf, membuf.length);
    }
    membuffer_destroy(&membuf);

    return ret;
}

/// \cond
// Using this variable to be able to modify it by gtests without recompile.
std::string web_server_content_language{WEB_SERVER_CONTENT_LANGUAGE};
/// \endcond

int http_MakeMessage(membuffer* buf, int http_major_version,
                     int http_minor_version, const char* fmt, ...) {
    // For format types look at the declaration of http_MakeMessage() in the
    // header file httpreadwrite.hpp.
    TRACE("Executing http_MakeMessage()")
    char c;
    char* s = NULL;
    size_t num;
    off_t bignum;
    size_t length;
    time_t* loc_time;
    time_t curr_time;
    struct tm date_storage;
    struct tm* date;
    const char* start_str;
    const char* end_str;
    int status_code;
    const char* status_msg;
    http_method_t method;
    const char* method_str;
    const char* url_str;
    const char* temp_str;
    uri_type url;
    uri_type* uri_ptr;
    int error_code = 0;
    va_list argp;
    char tempbuf[200];
    const char* weekday_str = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
    const char* month_str = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0"
                            "Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
    int rc = 0;

    memset(tempbuf, 0, sizeof(tempbuf));
    va_start(argp, fmt);
    while ((c = *fmt++) != 0) {
        if (c == 'E') {
            /* list of extra headers */
            UpnpListIter pos;
            UpnpListHead* head;
            UpnpExtraHeaders* extra;
            const DOMString resp;
            head = (UpnpListHead*)va_arg(argp, UpnpListHead*);
            if (head) {
                for (pos = UpnpListBegin(head); pos != UpnpListEnd(head);
                     pos = UpnpListNext(head, pos)) {
                    extra = (UpnpExtraHeaders*)pos;
                    resp = UpnpExtraHeaders_get_resp(extra);
                    if (resp) {
                        if (membuffer_append(buf, resp, strlen(resp)))
                            goto error_handler;
                        if (membuffer_append(buf, "\r\n", (size_t)2))
                            goto error_handler;
                    }
                }
            }
        } else if (c == 's') {
            /* C string */
            s = (char*)va_arg(argp, char*);
            assert(s);
            UpnpPrintf(UPNP_ALL, HTTP, __FILE__, __LINE__,
                       "Adding a string : %s\n", s);
            if (membuffer_append(buf, s, strlen(s)))
                goto error_handler;
        } else if (c == 'K') {
            /* Add Chunky header */
            if (membuffer_append(buf, "TRANSFER-ENCODING: chunked\r\n",
                                 strlen("Transfer-Encoding: chunked\r\n")))
                goto error_handler;
        } else if (c == 'G') {
            /* Add Range header */
            struct SendInstruction* RespInstr;
            RespInstr =
                (struct SendInstruction*)va_arg(argp, struct SendInstruction*);
            assert(RespInstr);
            /* connection header */
            if (membuffer_append(buf, RespInstr->RangeHeader,
                                 strlen(RespInstr->RangeHeader)))
                goto error_handler;
        } else if (c == 'b') {
            /* mem buffer */
            s = (char*)va_arg(argp, char*);
            UpnpPrintf(UPNP_ALL, HTTP, __FILE__, __LINE__,
                       "Adding a char Buffer starting with: %c\n", (int)s[0]);
            assert(s);
            length = (size_t)va_arg(argp, size_t);
            if (membuffer_append(buf, s, length))
                goto error_handler;
        } else if (c == 'c') {
            /* crlf */
            if (membuffer_append(buf, "\r\n", (size_t)2))
                goto error_handler;
        } else if (c == 'd') {
            /* integer */
            num = (size_t)va_arg(argp, int);
            rc = snprintf(tempbuf, sizeof(tempbuf), "%" PRIzu,
                          (unsigned long)num);
            if (rc < 0 || (unsigned int)rc >= sizeof(tempbuf) ||
                membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 'h') {
            /* off_t */
            bignum = (off_t)va_arg(argp, off_t);
            rc =
                snprintf(tempbuf, sizeof(tempbuf), "%" PRId64, (int64_t)bignum);
            if (rc < 0 || (unsigned int)rc >= sizeof(tempbuf) ||
                membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 't' || c == 'D') {
            /* date */
            if (c == 'D') {
                /* header */
                start_str = "DATE: ";
                end_str = "\r\n";
                curr_time = umock::sysinfo.time(NULL);
                loc_time = &curr_time;
            } else {
                /* date value only */
                start_str = end_str = "";
                loc_time = (time_t*)va_arg(argp, time_t*);
            }
            assert(loc_time);
            date = http_gmtime_r(loc_time, &date_storage);
            if (date == NULL)
                goto error_handler;
            rc = snprintf(tempbuf, sizeof(tempbuf),
                          "%s%s, %02d %s %d %02d:%02d:%02d GMT%s", start_str,
                          &weekday_str[date->tm_wday * 4], date->tm_mday,
                          &month_str[date->tm_mon * 4], date->tm_year + 1900,
                          date->tm_hour, date->tm_min, date->tm_sec, end_str);
            if (rc < 0 || (unsigned int)rc >= sizeof(tempbuf) ||
                membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 'L') {
            // Add CONTENT-LANGUAGE header only
            // if Accept-Language header is not empty
            // and
            // if web_server_content_language (aka WEB_SERVER_CONTENT_LANGUAGE)
            //    is not empty
            SendInstruction* RespInstr;
            RespInstr = (SendInstruction*)va_arg(argp, SendInstruction*);
            assert(RespInstr);
            if ((bool)strcmp(RespInstr->AcceptLanguageHeader, "") &&
                !web_server_content_language.empty() &&
                (bool)http_MakeMessage(
                    buf, http_major_version, http_minor_version, "ssc",
                    "CONTENT-LANGUAGE: ",
                    web_server_content_language.c_str()) != 0)
                goto error_handler;
        } else if (c == 'C') {
            if ((http_major_version > 1) ||
                (http_major_version == 1 && http_minor_version == 1)) {
                /* connection header */
                if (membuffer_append_str(buf, "CONNECTION: close\r\n"))
                    goto error_handler;
            }
        } else if (c == 'N') {
            /* content-length header */
            bignum = (off_t)va_arg(argp, off_t);
            assert(bignum >= 0);
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "shc", "CONTENT-LENGTH: ", bignum) != 0)
                goto error_handler;
            /* Add accept ranges */
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "sc", "Accept-Ranges: bytes") != 0)
                goto error_handler;
        } else if (c == 'S' || c == 'U') {
            /* SERVER or USER-AGENT header */
            temp_str = (c == 'S') ? "SERVER: " : "USER-AGENT: ";
            get_sdk_info(tempbuf, sizeof(tempbuf));
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "ss", temp_str, tempbuf) != 0)
                goto error_handler;
        } else if (c == 'X') {
            /* C string */
            s = (char*)va_arg(argp, char*);
            assert(s);
            if (membuffer_append_str(buf, "X-User-Agent: ") != 0)
                goto error_handler;
            if (membuffer_append(buf, s, strlen(s)) != 0)
                goto error_handler;
        } else if (c == 'R') {
            /* response start line */
            /*   e.g.: 'HTTP/1.1 200 OK' code */
            status_code = (int)va_arg(argp, int);
            assert(status_code > 0);
            rc = snprintf(tempbuf, sizeof(tempbuf), "HTTP/%d.%d %d ",
                          http_major_version, http_minor_version, status_code);
            /* str */
            status_msg = http_get_code_text(status_code);
            if (rc < 0 || (unsigned int)rc >= sizeof(tempbuf) ||
                http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "ssc", tempbuf, status_msg) != 0)
                goto error_handler;
        } else if (c == 'B') {
            /* body of a simple reply */
            status_code = (int)va_arg(argp, int);
            rc = snprintf(tempbuf, sizeof(tempbuf), "%s%d %s%s",
                          "<html><body><h1>", status_code,
                          http_get_code_text(status_code),
                          "</h1></body></html>");
            if (rc < 0 || (unsigned int)rc >= sizeof(tempbuf))
                goto error_handler;
            bignum = (off_t)strlen(tempbuf);
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "NTcs", bignum, /* content-length */
                                 "text/html",    /* content-type */
                                 tempbuf) != 0   /* body */
            )
                goto error_handler;
        } else if (c == 'Q') {
            /* request start line */
            /* GET /foo/bar.html HTTP/1.1\r\n */
            method = (http_method_t)va_arg(argp, int);
            method_str = method_to_str(method);
            url_str = (const char*)va_arg(argp, const char*);
            num = (size_t)va_arg(argp, size_t); /* length of url_str */
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "ssbsdsdc", method_str, /* method */
                                 " ", url_str, num,      /* url */
                                 " HTTP/", http_major_version, ".",
                                 http_minor_version) != 0)
                goto error_handler;
        } else if (c == 'q') {
            /* request start line and HOST header */
            method = (http_method_t)va_arg(argp, int);
            uri_ptr = (uri_type*)va_arg(argp, uri_type*);
            assert(uri_ptr);
            if (http_FixUrl(uri_ptr, &url) != 0) {
                error_code = UPNP_E_INVALID_URL;
                goto error_handler;
            }
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "Q"
                                 "sbc",
                                 method, url.pathquery.buff, url.pathquery.size,
                                 "HOST: ", url.hostport.text.buff,
                                 url.hostport.text.size) != 0)
                goto error_handler;
        } else if (c == 'T') {
            /* content type header */
            temp_str = (const char*)va_arg(
                argp, const char*); /* type/subtype format */
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                                 "ssc", "CONTENT-TYPE: ", temp_str) != 0)
                goto error_handler;
        } else {
            assert(0);
        }
    }
    goto ExitFunction;

error_handler:
    /* Default is out of memory error. */
    if (!error_code)
        error_code = UPNP_E_OUTOF_MEMORY;
    membuffer_destroy(buf);

ExitFunction:
    va_end(argp);
    return error_code;
}

void http_CalcResponseVersion(int request_major_vers, int request_minor_vers,
                              int* response_major_vers,
                              int* response_minor_vers) {
    if ((request_major_vers > 1) ||
        (request_major_vers == 1 && request_minor_vers >= 1)) {
        *response_major_vers = 1;
        *response_minor_vers = 1;
    } else {
        *response_major_vers = request_major_vers;
        *response_minor_vers = request_minor_vers;
    }
}


int http_OpenHttpGetEx(const char* url_str, void** Handle, char** contentType,
                       int* contentLength, int* httpStatus, int lowRange,
                       int highRange, int timeout) {
    int http_error_code;
    memptr ctype;
    SOCKET tcp_connection;
    size_t sockaddr_len;
    membuffer request;
    http_connection_handle_t* handle = NULL;
    uri_type url;
    parse_status_t status;
    int errCode = UPNP_E_SUCCESS;
    struct SendInstruction rangeBuf;
    int rc = 0;

    membuffer_init(&request);

    do {
        /* Checking Input parameters */
        if (!url_str || !Handle || !contentType || !httpStatus) {
            errCode = UPNP_E_INVALID_PARAM;
            break;
        }
        /* Initialize output parameters */
        *httpStatus = 0;
        *Handle = handle;
        *contentType = NULL;
        *contentLength = 0;
        if (lowRange > highRange) {
            errCode = UPNP_E_INTERNAL_ERROR;
            break;
        }
        memset(&rangeBuf, 0, sizeof(rangeBuf));
        rc = snprintf(rangeBuf.RangeHeader, sizeof(rangeBuf.RangeHeader),
                      "Range: bytes=%d-%d\r\n", lowRange, highRange);
        if (rc < 0 || (unsigned int)rc >= sizeof(rangeBuf.RangeHeader))
            break;
        membuffer_init(&request);
        errCode = MakeGetMessageEx(url_str, &request, &url, &rangeBuf);
        if (errCode != UPNP_E_SUCCESS)
            break;
        handle =
            (http_connection_handle_t*)malloc(sizeof(http_connection_handle_t));
        if (!handle) {
            errCode = UPNP_E_OUTOF_MEMORY;
            break;
        }
        memset(handle, 0, sizeof(*handle));
        parser_response_init(&handle->response, HTTPMETHOD_GET);
        tcp_connection = umock::sys_socket_h.socket(
            (int)url.hostport.IPaddress.ss_family, SOCK_STREAM, 0);
        if (tcp_connection == INVALID_SOCKET) {
            errCode = UPNP_E_SOCKET_ERROR;
            free(handle);
            break;
        }
        if (sock_init(&handle->sock_info, tcp_connection) != UPNP_E_SUCCESS) {
            sock_destroy(&handle->sock_info, SD_BOTH);
            errCode = UPNP_E_SOCKET_ERROR;
            free(handle);
            break;
        }
        sockaddr_len = url.hostport.IPaddress.ss_family == AF_INET6
                           ? sizeof(struct sockaddr_in6)
                           : sizeof(struct sockaddr_in);
        errCode = umock::pupnp_httprw.private_connect(
            handle->sock_info.socket,
            (struct sockaddr*)&(url.hostport.IPaddress),
            (socklen_t)sockaddr_len);
        if (errCode == -1) {
            sock_destroy(&handle->sock_info, SD_BOTH);
            errCode = UPNP_E_SOCKET_CONNECT;
            free(handle);
            break;
        }
        /* send request */
        errCode = http_SendMessage(&handle->sock_info, &timeout, "b",
                                   request.buf, request.length);
        if (errCode != UPNP_E_SUCCESS) {
            sock_destroy(&handle->sock_info, SD_BOTH);
            free(handle);
            break;
        }
        if (ReadResponseLineAndHeaders(&handle->sock_info, &handle->response,
                                       &timeout,
                                       &http_error_code) != (int)PARSE_OK) {
            errCode = UPNP_E_BAD_RESPONSE;
            free(handle);
            break;
        }
        status = parser_get_entity_read_method(&handle->response);
        if (status != (parse_status_t)PARSE_CONTINUE_1 &&
            status != (parse_status_t)PARSE_SUCCESS) {
            errCode = UPNP_E_BAD_RESPONSE;
            free(handle);
            break;
        }
        *httpStatus = handle->response.msg.status_code;
        errCode = UPNP_E_SUCCESS;

        if (!httpmsg_find_hdr(&handle->response.msg, HDR_CONTENT_TYPE, &ctype))
            /* no content-type */
            *contentType = NULL;
        else
            *contentType = ctype.buf;
        if (handle->response.position == (parser_pos_t)POS_COMPLETE)
            *contentLength = 0;
        else if (handle->response.ent_position == ENTREAD_USING_CHUNKED)
            *contentLength = UPNP_USING_CHUNKED;
        else if (handle->response.ent_position == ENTREAD_USING_CLEN)
            *contentLength = (int)handle->response.content_length;
        else if (handle->response.ent_position == ENTREAD_UNTIL_CLOSE)
            *contentLength = UPNP_UNTIL_CLOSE;
        *Handle = handle;
    } while (0);

    membuffer_destroy(&request);

    return errCode;
}

/* 'info' should have a size of at least 100 bytes */
void get_sdk_info(char* info, size_t infoSize) {
    TRACE("Executing get_sdk_info().")
#ifdef UPNP_ENABLE_UNSPECIFIED_SERVER
    snprintf(info, infoSize, "Unspecified, UPnP/1.0, Unspecified\r\n");
#else /* UPNP_ENABLE_UNSPECIFIED_SERVER */
#ifdef _WIN32
    snprintf(info, infoSize,
             "UPnP/1.0, Portable SDK for UPnP devices/" UPNP_VERSION_STRING
             " on windows\r\n");
#else
    struct utsname sys_info;

    int ret_code = umock::sysinfo.uname(&sys_info);
    if (ret_code == -1)
        snprintf(info, infoSize,
                 "Unspecified, UPnP/1.0, Portable SDK for UPnP "
                 "devices/" UPNP_VERSION_STRING "\r\n");
    else
        snprintf(info, infoSize,
                 "%s/%s, UPnP/1.0, Portable SDK for UPnP "
                 "devices/" UPNP_VERSION_STRING "\r\n",
                 sys_info.sysname, sys_info.release);
#endif
#endif /* UPNP_ENABLE_UNSPECIFIED_SERVER */
}
