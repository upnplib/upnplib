#ifdef COMPA_HAVE_DEVICE_DESCRIPTION

#ifndef COMPA_URLCONFIG_HPP
#define COMPA_URLCONFIG_HPP
/* ************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-07
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
// Last compare with pupnp original source file on 2022-11-10, ver 1.14.14
/*!
 * \file
 * \brief Configure the full URL for the description document.
 * \ingroup Description-device
 */

#include <upnp.hpp>

/*!
 * \brief Configure the full URL for the description document.
 *
 * Create the URL document and add alias, description information. The doc is
 * added to the web server to be served using the given alias.
 *
 * \note The function is available only if the web server was enabled on
 * compiling.
 *
 * \returns
 *  On success: UPNP_E_SUCCESS\n
 *  On error: UPNP_E_OUTOF_MEMORY - Default Error
 */
int configure_urlbase(
    /*! [in,out] IXML Description document (dom document whose urlbase is to be
                 modified). */
    IXML_Document* doc,
    /*! [in] Socket address object providing the IP address and port information
             (ip address and port of the miniserver). */
    const sockaddr* serverAddr,
    /*! [in] String containing the alias (a name to be used for the temp,
             e.g.:"foo.xml"). */
    const char* alias,
    /*! [in] Time when the XML document was downloaded. */
    time_t last_modified,
    /*! [out] Buffer to hold the URL of the document. */
    char docURL[LINE_SIZE]);

#endif /* COMPA_URLCONFIG_HPP */
#endif // COMPA_HAVE_DEVICE_DESCRIPTION
