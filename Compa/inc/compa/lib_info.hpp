#ifndef COMPA_INIT_HPP
#define COMPA_INIT_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-08
/*!
 * \file
 * \ingroup compa-Operating
 * \brief Information about the library.
 */

#include <upnplib/visibility.hpp>
/// \cond
#include <string>
/// \endcond

/*!
 * \brief Human readable version description of the library.
 *
 * \returns String describing the library version.
 */
UPNPLIB_API std::string libversion();

#endif // COMPA_INIT_HPP
