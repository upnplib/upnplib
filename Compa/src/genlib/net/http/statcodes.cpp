/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-14
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
/*!
 * \file
 * \brief HTTP status codes
 *
 * This file defines status codes, buffers to store the status messages and
 * functions to manipulate those buffers.
 */

#include <config.hpp>
#include <statcodes.hpp>
#include <upnp.hpp>

/// \cond
#include <string.h>
/// \endcond

namespace {

/// \cond
constexpr int NUM_1XX_CODES{2};
const char* Http1xxCodes[NUM_1XX_CODES];
const char* Http1xxStr = "Continue\0"
                         "Switching Protocols\0";

constexpr int NUM_2XX_CODES{7};
const char* Http2xxCodes[NUM_2XX_CODES];
const char* Http2xxStr = "OK\0"
                         "Created\0"
                         "Accepted\0"
                         "Non-Authoratative Information\0"
                         "No Content\0"
                         "Reset Content\0"
                         "Partial Content\0";

constexpr int NUM_3XX_CODES{8};
const char* Http3xxCodes[NUM_3XX_CODES];
const char* Http3xxStr = "Multiple Choices\0"
                         "Moved Permanently\0"
                         "Found\0"
                         "See Other\0"
                         "Not Modified\0"
                         "Use Proxy\0"
                         "\0"
                         "Temporary Redirect\0";

constexpr int NUM_4XX_CODES{18};
const char* Http4xxCodes[NUM_4XX_CODES];
const char* Http4xxStr = "Bad Request\0"
                         "Unauthorized\0"
                         "Payment Required\0"
                         "Forbidden\0"
                         "Not Found\0"
                         "Method Not Allowed\0"
                         "Not Acceptable\0"
                         "Proxy Authentication Required\0"
                         "Request Timeout\0"
                         "Conflict\0"
                         "Gone\0"
                         "Length Required\0"
                         "Precondition Failed\0"
                         "Request Entity Too Large\0"
                         "Request-URI Too Long\0"
                         "Unsupported Media Type\0"
                         "Requested Range Not Satisfiable\0"
                         "Expectation Failed\0";

constexpr int NUM_5XX_CODES{11};
const char* Http5xxCodes[NUM_5XX_CODES];
const char* Http5xxStr = "Internal Server Error\0"
                         "Not Implemented\0"
                         "Bad Gateway\0"
                         "Service Unavailable\0"
                         "Gateway Timeout\0"
                         "HTTP Version Not Supported\0"
                         "Variant Also Negotiates\0"
                         "Insufficient Storage\0"
                         "Loop Detected\0"
                         "\0"
                         "Not Extended\0";

bool gInitialized{false};
/// \endcond

/*!
 * \name Scope restricted to file
 * @{
 */

/*!
 * \brief Initializing table representing an array of string pointers.
 *
 * Initialize with the individual strings that are comprised in the input const
 * char* encoded_str parameter.
 */
UPNP_INLINE void init_table( //
    const char* encoded_str, ///< [in] Status code encoded string.
    const char* table[],     /*!< [out] Table to store the encoded status code
                                        strings. */
    int tbl_size             ///< [in] Size of the table.
) {
    int i;
    const char* s = encoded_str;

    for (i = 0; i < tbl_size; i++) {
        table[i] = s;
        s += strlen(s) + (size_t)1; /* next entry */
    }
}

/*!
 * \brief Initializing tables with HTTP strings and different HTTP codes.
 * \details Show graphs...
 */
UPNP_INLINE void init_tables() {
    init_table(Http1xxStr, Http1xxCodes, NUM_1XX_CODES);
    init_table(Http2xxStr, Http2xxCodes, NUM_2XX_CODES);
    init_table(Http3xxStr, Http3xxCodes, NUM_3XX_CODES);
    init_table(Http4xxStr, Http4xxCodes, NUM_4XX_CODES);
    init_table(Http5xxStr, Http5xxCodes, NUM_5XX_CODES);

    gInitialized = true; /* mark only after complete */
}

/// @} // Scope restricted to file
} // anonymous namespace


/*!
 * \brief Return the right status message based on the passed in int statusCode
 * input parameter.
 *
 * \returns Pointer to the status message string.
 */
const char* http_get_code_text(
    int statusCode /*!< [in] Status code based on which the status table and
                             status message is returned. */
) {
    int index;
    int table_num;

    if (!gInitialized) {
        init_tables();
    }

    if (statusCode < 100 || statusCode >= 600) {
        return NULL;
    }

    index = statusCode % 100;
    table_num = statusCode / 100;

    if (table_num == 1 && index < NUM_1XX_CODES) {
        return Http1xxCodes[index];
    }

    if (table_num == 2 && index < NUM_2XX_CODES) {
        return Http2xxCodes[index];
    }

    if (table_num == 3 && index < NUM_3XX_CODES) {
        return Http3xxCodes[index];
    }

    if (table_num == 4 && index < NUM_4XX_CODES) {
        return Http4xxCodes[index];
    }

    if (table_num == 5 && index < NUM_5XX_CODES) {
        return Http5xxCodes[index];
    }

    return NULL;
}
