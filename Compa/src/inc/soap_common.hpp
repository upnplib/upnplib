#ifndef COMPA_SOAP_COMMON_HPP
#define COMPA_SOAP_COMMON_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-15
/*!
 * \file
 * \brief Common SOAP declarations used for SOAP Devices and SOAP Control
 * Points.
 */

#include <config.hpp>
#if (EXCLUDE_SOAP == 0) || defined(DOXYGEN_RUN)

/// \brief Common SOAP constant string specifying the content type header.
inline constexpr char ContentTypeHeader[]{
    "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"};

#endif /* EXCLUDE_SOAP */
#endif // COMPA_SOAP_COMMON_HPP
