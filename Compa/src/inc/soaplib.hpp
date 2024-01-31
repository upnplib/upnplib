#ifndef COMPA_SOAPLIB_HPP
#define COMPA_SOAPLIB_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-01
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
/*!
 * \file
 * \brief SOAP module API to be called in UPnP-SDK API. No details
 */

#include <sock.hpp>

/*!
 * \brief This is a callback called by minisever.
 *
 * This is called after receiving the request from the control point. The
 * function will start processing the request. It calls handle_invoke_action()
 * to handle the SOAP action.
 */
void soap_device_callback(
    /*! [in] Parsed request received by the device. */
    http_parser_t* parser,
    /*! [in] HTTP request. */
    http_message_t* request,
    /*! [in,out] Socket info. */
    SOCKINFO* info);

/*!
 * \brief This function is called by UPnP API to send the SOAP action request.
 *
 * It waits till it gets the response from the device pass the response to the
 * API layer.
 *
 * \returns
 * On success: UPNP_E_SUCCESS\n
 * On error: appropriate error
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
 * On error: appropriate error
 */
int SoapSendActionEx(        //
    char* ActionURL,         ///< [in] device contrl URL.
    char* ServiceType,       ///< [in] device service type.
    IXML_Document* Header,   ///< [in] Soap header.
    IXML_Document* ActNode,  ///< [in] SOAP action node (SOAP body).
    IXML_Document** RespNode ///< [out] SOAP response node.
);

/*!
 * \brief This function creates a status variable query message send it to the
 * specified URL. It also collect the response.
 *
 * \returns
 * On success: ???\n
 * On error: ???
 */
int SoapGetServiceVarStatus(
    char* ActionURL,   ///< [in] Address to send this variable query message.
    DOMString VarName, ///< [in] Name of the variable.
    DOMString* StVar   ///< [out] Output value.
);

/// \brief Global constant string specifying the content type header.
extern const char* ContentTypeHeader;

#endif /* COMPA_SOAPLIB_HPP */
