#ifndef UPNPLIB_GLOBAL_HPP
#define UPNPLIB_GLOBAL_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-20
/*!
 * \file
 * \brief Global used constants and variables.
 */

// Due to the global nature of this header file additional #include statements
// should be taken with great care. They are included in nearly all other
// compile units.
#include <upnplib/global.ipp>

namespace upnplib {

// Global variables
// ================
/*!
 * \brief Switch to enable verbose (debug) output.
 *
 * This flag is only modified by user intervention, e.g. on the command line or
 * with an environment variable but never modified by the production code. Only
 * Unit tests may toggle the switch under test.
 */
UPNPLIB_EXTERN bool g_dbug;

} // namespace upnplib

#endif // UPNPLIB_GLOBAL_HPP
