#ifndef COMPA_SOAP_CTRLPT_HPP
#define COMPA_SOAP_CTRLPT_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-15
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
// Last compare with ./Pupnp source file on 2024-02-14, ver 1.14.18
/*!
 * \file
 * \brief SOAP declarations for Control Points using SOAP.
 */

#include <ixml.hpp>

#ifndef COMPA_INTERNAL_CONFIG_HPP
#error "No or wrong config.hpp header file included."
#endif

#if (EXCLUDE_SOAP == 0) || defined(DOXYGEN_RUN)

/*!
 * \brief This function is called by UPnP API to send the SOAP action request.
 *
 * It waits till it gets the response from the device pass the response to the
 * API layer.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error:
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_ACTION
 *  - UPNP_E_INVALID_URL
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_CONNECT
 *  - UPNP_E_FILE_READ_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 *  - UPNP_E_BAD_HTTPMSG
 *  - may have additional error codes
 */
int SoapSendAction(               //
    char* action_url,             ///< [in] device contrl URL.
    char* service_type,           ///< [in] device service type.
    IXML_Document* action_node,   ///< [in] SOAP action node.
    IXML_Document** response_node ///< [out] SOAP response node.
);

/*!
 * \brief This extended function is called by UPnP API to send the SOAP action
 * request.
 *
 * The function waits till it gets the response from the device pass the
 * response to the API layer. This action is similar to the SoapSendAction()
 * with only difference that it allows users to pass the SOAP header along the
 * SOAP body (soap action request).
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error:
 *  - UPNP_E_BAD_RESPONSE
 *  - UPNP_E_OUTOF_MEMORY
 *  - UPNP_E_INVALID_ACTION
 *  - UPNP_E_INVALID_URL
 *  - UPNP_E_SOCKET_ERROR
 *  - UPNP_E_SOCKET_CONNECT
 *  - UPNP_E_FILE_READ_ERROR
 *  - UPNP_E_TIMEDOUT
 *  - UPNP_E_SOCKET_WRITE
 *  - UPNP_E_BAD_HTTPMSG
 *  - SOAP_ACTION_RESP
 *  - SOAP_VAR_RESP
 *  - SOAP_VAR_RESP_ERROR
 *  - SOAP_ACTION_RESP_ERROR
 *  - HTTP error codes >400
 */
int SoapSendActionEx(             //
    char* action_url,             ///< [in] device contrl URL.
    char* service_type,           ///< [in] device service type.
    IXML_Document* header,        ///< [in] Soap header.
    IXML_Document* action_node,   ///< [in] SOAP action node (SOAP body).
    IXML_Document** response_node ///< [out] SOAP response node.
);

/*!
 * \brief This function creates a status variable query message send it to the
 * specified URL. It also collect the response.
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
 *  - UPNP_E_BAD_RESPONSE
 *  - SOAP_ACTION_RESP
 *  - SOAP_VAR_RESP
 *  - SOAP_VAR_RESP_ERROR
 *  - SOAP_ACTION_RESP_ERROR
 *  - HTTP error codes >400
 *
 * \todo Returned error message IDs are ambiguous. Fix it.
 */
int SoapGetServiceVarStatus(
    char* action_url,    ///< [in] Address to send this variable query message.
    DOMString var_name,  ///< [in] Name of the variable.
    DOMString* var_value ///< [out] Output value.
);

#endif /* EXCLUDE_SOAP */
#endif /* COMPA_SOAP_CTRLPT_HPP */
