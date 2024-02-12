#ifndef COMPA_GENLIB_NET_URI_HPP
#define COMPA_GENLIB_NET_URI_HPP
/* *****************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-12
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
 * ****************************************************************************/
/*!
 * \file
 * \brief Modify and parse URIs.
 */

#include <upnplib/visibility.hpp>

/// \cond
#include "winsock2.h" // For different platforms: don't use <winsock2.h>

#include <cctype>
#include <cstring>

#ifndef _WIN32
#include <netdb.h> /* for struct addrinfo */
#endif
/// \endcond

#ifdef _WIN32
#define strncasecmp strnicmp
#else
/* Other systems have strncasecmp */
#endif

/// Yet another success code.
inline constexpr int HTTP_SUCCESS{1};

/// Type of the "path" part of the URI.
enum pathType { ABS_PATH, REL_PATH, OPAQUE_PART };

#if defined(_WIN32) || defined(DOXYGEN_RUN)
/*! \brief Need this for WIN32. There is a conflict with other symbols.
 *
 * Default is\n`enum uriType { ABSOLUTE, RELATIVE };` */
enum uriType { absolute, relative };
#else
enum uriType { ABSOLUTE, RELATIVE };
#endif

/*!
 * \brief Buffer used in parsinghttp messages, urls, etc. Generally this simply
 * holds a pointer into a larger array.
 */
struct token {
    const char* buff; ///< Buffer
    size_t size;      ///< Size of the buffer
};

/*!
 * \brief Represents a host port, e.g. "127.127.0.1:80".
 */
struct hostport_type {
    token text; ///< Pointing to the full host:port string representation.
    sockaddr_storage IPaddress; ///< Network socket address.
};

/*!
 * \brief Represents a URI used in parse_uri and elsewhere.
 */
struct uri_type {
    /// @{
    /// \brief Member variable
    uriType type;
    token scheme;
    pathType path_type;
    token pathquery;
    token fragment;
    hostport_type hostport;
    /// @}
};

/*!
 * \brief Represents a list of URLs as in the "callback" header of SUBSCRIBE
 * message in GENA.
 */
struct URL_list {
    size_t size;          ///< size
    char* URLs;           ///< Dynamic memory for all urls, delimited by `<>`
    uri_type* parsedURLs; ///< parsed URLs
};

/*!
 * \brief Replaces one single escaped character within a string with its
 * unescaped version.
 *
 * This is spezified in <a href="http://www.ietf.org/rfc/rfc2396.txt">RFC 2396
 * (explaining URIs)</a>. The index must exactly point to the \b '\%'
 * character, otherwise the function will return unsuccessful. Size of array is
 * NOT checked (MUST be checked by caller).
 *
 * \note This function modifies the string and the max size. If the sequence is
 * an escaped sequence it is replaced, the other characters in the string are
 * shifted over, and NULL characters are placed at the end of the string.
 *
 * \returns
 *   1 - if an escaped character was converted\n
 *   0 - otherwise
 */
UPNPLIB_API int replace_escaped(
    /*! [in,out] String of characters. */
    char* in,
    /// [in] Index at which to start checking the characters; must point to '%'.
    size_t index,
    /*! [in,out] Maximal size of the string buffer will be reduced by 2 if a
       character is converted. */
    size_t* max);

/*!
 * \brief Copies one URL_list into another.
 *
 * This includes dynamically allocating the out->URLs field (the full string),
 * and the structures used to hold the parsedURLs. This memory MUST be freed
 * by the caller through: free_URL_list(&out).
 *
 * \returns
 *  On success: HTTP_SUCCESS\n
 *  On error: UPNP_E_OUTOF_MEMORY - On Failure to allocate memory.
 */
UPNPLIB_API int copy_URL_list(
    /*! [in] Source URL list. */
    URL_list* in,
    /*! [out] Destination URL list. */
    URL_list* out);

/*!
 * \brief Frees the memory associated with a URL_list.
 *
 * Frees the dynamically allocated members of of list. Does NOT free the
 * pointer to the list itself ( i.e. does NOT free(list)).
 */
UPNPLIB_API void free_URL_list(
    /*! [in] URL list object. */
    URL_list* list);

#if defined(DEBUG) || defined(DOXYGEN_RUN)
/*! \brief Function useful in debugging for printing a parsed uri.
 * \details This is only available when compiled with DEBUG enabled. */
void print_uri(uri_type* in ///< [in] URI object to print.
);
#else
#define print_uri(in)                                                          \
    do {                                                                       \
    } while (0)
#endif

#if defined(DEBUG) || defined(DOXYGEN_RUN)
/*! \brief Function useful in debugging for printing a token.
 * \details This is only available when compiled with DEBUG enabled. */
void print_token( //
    token* in     ///< [in] Token object to print.
);
#else
#define print_token(in)                                                        \
    do {                                                                       \
    } while (0)
#endif

/*!
 * \brief Compares buffer in the token object with the buffer in in2 case
 * insensitive.
 *
 * \return
 *  \li < 0, if string1 is less than string2.
 *  \li == 0, if string1 is identical to string2 .
 *  \li > 0, if string1 is greater than string2.
 */
UPNPLIB_API int token_string_casecmp(
    /*! [in] Token object whose buffer is to be compared. */
    token* in1,
    /*! [in] String of characters to compare with. */
    const char* in2);

/*!
 * \brief Compares two tokens.
 *
 * \return
 *  \li < 0, if string1 is less than string2.
 *  \li == 0, if string1 is identical to string2 .
 *  \li > 0, if string1 is greater than string2.
 */
UPNPLIB_API int token_cmp(
    /*! [in] First token object whose buffer is to be compared. */
    token* in1,
    /*! [in] Second token object used for the comparison. */
    token* in2);

/*!
 * \brief Removes http escaped characters such as: "%20" and replaces them with
 * their character representation.
 *
 * For example: "hello%20foo" -> "hello foo". The input IS MODIFIED in place
 * (shortened). Extra characters are replaced with \b NULL.
 *
 * \returns UPNP_E_SUCCESS.
 */
UPNPLIB_API int remove_escaped_chars(
    /*! [in,out] String of characters to be modified. */
    char* in,
    /*! [in,out] Size limit for the number of characters. */
    size_t* size);

/*!
 * \brief Removes ".", and ".." from a path.
 *
 * If a ".." can not be resolved (i.e. the .. would go past the root of the
 * path) an error is returned. The input IS modified in place. This function
 * directly implements the "Remove Dot Segments" algorithm described in RFC
 * 3986 section 5.2.4.
 *
 * \verbatim
Examples:
 uchar path[30]="/../hello";
 uremove_dots(path, strlen(path)) -> UPNP_E_INVALID_URL
 uchar path[30]="/./hello";
 uremove_dots(path, strlen(path)) -> UPNP_E_SUCCESS,
 uin = "/hello"
 uchar path[30]="/./hello/foo/../goodbye" ->
 uUPNP_E_SUCCESS, in = "/hello/goodbye"
\endverbatim
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY - On failure to allocate memory.
 *  - UPNP_E_INVALID_URL - Failure to resolve URL.
 */
UPNPLIB_API int remove_dots(
    /*! [in] String of characters from which "dots" have to be removed. */
    char* buf,
    /*! [in] Size limit for the number of characters. */
    size_t size);

/*!
 * \brief Resolves a relative url with a base url.
 *
 * - If the base_url is a \b nullptr, then a copy of the rel_url is passed back.
 * - If the rel_url is absolute then a copy of the rel_url is passed back.
 * - If neither the base nor the rel_url are absolute then a \b nullptr is
 *   returned.
 * - Otherwise it tries and resolves the relative url with the base as
 *   described in <a href="http://www.ietf.org/rfc/rfc2396.txt">RFC 2396
 *   (explaining URIs)</a>.
 *
 * The resolution of '..' is NOT implemented, but '.' is resolved.
 *
 * \returns
 *  Pointer to a new with malloc dynamically allocated full URL or a \b nullptr.
 */
UPNPLIB_API char* resolve_rel_url(
    /*! [in] Base URL. */
    char* base_url,
    /*! [in] Relative URL. */
    char* rel_url);

/*!
 * \brief Parses a uri as defined in <a
 * href="http://www.ietf.org/rfc/rfc2396.txt"> RFC 2396 (explaining URIs)</a>.
 *
 * Handles absolute, relative, and opaque uris. Parses into the following
 * pieces: scheme, hostport, pathquery, fragment (host with port and path with
 * query are treated as one token). Strings in output uri_type are treated as
 * token with character chain and size. They are not null ('\0') terminated.
 *
 * Caller should check for the pieces they require.
 *
 * \returns
 *  On success: HTTP_SUCCESS\n
 *  On error: UPNP_E_INVALID_URL
 */
UPNPLIB_API int parse_uri(
    /*! [in] Character string containing uri information to be parsed. */
    const char* in,
    /*! [in] Number of characters (strlen()) of the input string. */
    size_t max,
    /*! [out] Output parameter which will have the parsed uri information.
     */
    uri_type* out);

/*!
 * \brief
 *
 * \return
 */
int parse_token(
    /*! [in] . */
    char* in,
    /*! [out] . */
    token* out,
    /*! [in] . */
    int max_size);

#endif /* COMPA_GENLIB_NET_URI_HPP */
