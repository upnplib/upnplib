#ifdef COMPA_HAVE_MINISERVER

#ifndef COMPA_MINISERVER_HPP
#define COMPA_MINISERVER_HPP
/**************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-05-24
 * Copied from pupnp ver 1.14.15.
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
 * \ingroup compa-Addressing
 * \brief Manage "Step 0: Addressing" of the UPnP+™ specification.
 */

#include <httpparser.hpp>
#include <sock.hpp>
#include <upnplib/socket.hpp>

/// \cond
#include <cstdint> // for uint16_t
/// \endcond

/// \brief Provides sockets for all network communications.
struct MiniServerSockArray {
    /*! \brief IPv4 socket for listening for miniserver requests. */
    SOCKET miniServerSock4;
    /*! \brief IPv6 LLA Socket for listening for miniserver requests. */
    SOCKET miniServerSock6;
    /*! \brief IPv6 ULA or GUA Socket for listening for miniserver requests. */
    SOCKET miniServerSock6UlaGua;
    /*! \brief Datagram Socket for stopping miniserver. */
    SOCKET miniServerStopSock;
    /*! \brief IPv4 SSDP datagram Socket for incoming advertisments and search
     * requests. */
    SOCKET ssdpSock4;
    /*! \brief IPv6 LLA SSDP Socket for incoming advertisments and search
     * requests. */
    SOCKET ssdpSock6;
    /*! \brief IPv6 ULA or GUA SSDP Socket for incoming advertisments and search
     * requests. */
    SOCKET ssdpSock6UlaGua;
    /*! \brief Corresponding port to miniServerStopSock. This is set with
     * miniStopSockPort but never used. */
    in_port_t stopPort;
    /*! \brief Corresponding port to miniServerSock4 */
    in_port_t miniServerPort4;
    /*! \brief Corresponding port to miniServerSock6 */
    in_port_t miniServerPort6;
    /*! \brief Corresponding port to miniServerSock6UlaGua */
    in_port_t miniServerPort6UlaGua;
#ifdef COMPA_HAVE_CTRLPT_SSDP
    /*! \name Only with Client (control point) Module.
     * \todo Move this to control point SSDP
     * @{ */
    /*! \brief IPv4 SSDP socket for sending search requests and receiving search
     * replies */
    SOCKET ssdpReqSock4;
    /*! \brief IPv6 SSDP socket for sending search requests and receiving search
     * replies */
    SOCKET ssdpReqSock6;
    /// @}
#endif
    upnplib::CSocket* MiniSvrSock6LlaObj{nullptr};
    upnplib::CSocket* MiniSvrSock6UadObj{nullptr};
    upnplib::CSocket* MiniSvrSock4Obj{nullptr};
};

/*! \brief For a miniserver callback function. */
typedef void (*MiniServerCallback)(http_parser_t* parser,
                                   http_message_t* request, SOCKINFO* info);

/// \brief HTTP server callback
inline MiniServerCallback gGetCallback{nullptr};

/*!
 * \brief Set SOAP Callback.
 */
#ifdef COMPA_HAVE_DEVICE_SOAP
void SetSoapCallback(
    /*! [in] SOAP Callback to be invoked. */
    MiniServerCallback callback);
#endif

/*!
 * \brief Set GENA Callback.
 */
#ifdef COMPA_HAVE_DEVICE_GENA
void SetGenaCallback(
    /*! [in] GENA Callback to be invoked. */
    MiniServerCallback callback);
#endif

/*!
 * \brief Initialize the sockets functionality for the Miniserver.
 *
 * Initialize a thread pool job to run the MiniServer and the job to the thread
 * pool. If a listening port is specified to 0, then the port number is
 * dynamically picked. Use timer mechanism to start the MiniServer, failure to
 * meet the allowed delay aborts the attempt to launch the MiniServer.
 *
 * The three parameter are only used if we have also a webserver built in. The
 * miniserver does not need them.
 *
 * I use a simple random number generator for unspecified port numbers. This is
 * needed because I do not reuse addresses before TIME_WAIT has expired (socket
 * option SO_REUSEADDR = false). I must use different socket addresses to be
 * able to restart a device immediately without TIME_WAIT and that is already
 * given with different port numbers on the same ip address.
 *
 * \returns
 *  - On success: UPNP_E_SUCCESS\n
 *  - On error: UPNP_E_XXX
 */
int StartMiniServer(
    /*! [in,out] Port on which the server listens for incoming IPv4
     * connections. */
    in_port_t* listen_port4,
    /*! [in,out] Port on which the server listens for incoming IPv6 LLA
       connections. */
    in_port_t* listen_port6,
    /*! [in,out] Port on which the server listens for incoming IPv6 ULA or GUA
       connections. */
    in_port_t* listen_port6UlaGua);

/*!
 * \brief Stop and Shutdown the MiniServer and free socket resources.
 *
 * \returns Always returns 0.
 */
int StopMiniServer();

#endif /* COMPA_MINISERVER_HPP */
#endif // COMPA_HAVE_MINISERVER
