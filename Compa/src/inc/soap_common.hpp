#if defined(COMPA_HAVE_CTRLPT_SOAP) || defined(COMPA_HAVE_DEVICE_SOAP)

#ifndef COMPA_SOAP_COMMON_HPP
#define COMPA_SOAP_COMMON_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-03
/*!
 * \file
 * \brief Common SOAP declarations used for SOAP Devices and SOAP Control
 * Points.
 */

/// \brief Common SOAP constant string specifying the content type header.
inline constexpr char ContentTypeHeader[]{
    "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"};

#endif // COMPA_SOAP_COMMON_HPP
#endif /* COMPA_HAVE_CTRLPT_SOAP || COMPA_HAVE_DEVICE_SOAP */
