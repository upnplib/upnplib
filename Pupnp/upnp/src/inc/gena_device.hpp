/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2022-02-19
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

#ifndef UPNPLIB_GENA_DEVICE_HPP
#define UPNPLIB_GENA_DEVICE_HPP

/*!
 * \file
 */

#include "httpparser.hpp"
#include "sock.hpp"

/*!
 * \brief Handles a subscription request from a ctrl point. The socket is not
 * closed on return.
 */
void gena_process_subscription_request(
    /*! [in] Socket info of the device. */
    SOCKINFO* info,
    /*! [in] Subscription request from the control point. */
    http_message_t* request);

/*!
 * \brief Handles a subscription renewal request from a ctrl point.
 * The connection is not destroyed on return.
 */
void gena_process_subscription_renewal_request(
    /*! [in] Socket info of the device. */
    SOCKINFO* info,
    /*! [in] Subscription renewal request from the control point. */
    http_message_t* request);

/*!
 * \brief Handles a subscription cancellation request from a ctrl point.
 * The connection is not destroyed on return.
 */
void gena_process_unsubscribe_request(
    /*! [in] Socket info of the device. */
    SOCKINFO* info,
    /*! [in] UNSUBSCRIBE request from the control point. */
    http_message_t* request);

#endif /* UPNPLIB_GENA_DEVICE_HPP */
