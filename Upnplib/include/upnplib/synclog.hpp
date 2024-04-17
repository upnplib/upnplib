#ifndef UPNPLIB_SYNCLOG_HPP
#define UPNPLIB_SYNCLOG_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-04-18
/*!
 * \file
 * \brief Define macro for synced logging to the console for detailed info and
 * debug.
 */

#include <upnplib/visibility.hpp>
/// \cond
#include <string>
#include <iostream>
#ifndef __APPLE__
#include <syncstream>
#endif

namespace upnplib {
UPNPLIB_EXTERN bool g_dbug;
} // namespace upnplib


// clang-format off
// Debug output messages with some that can be enabled during runtime.
// -------------------------------------------------------------------
// __PRETTY_FUNCTION__ is defined for POSIX so we have it there for the
// function signature to output for information. On MSC_VER it is named
// __FUNCTION__.
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
// or more verbose: #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// This is intended to be used as:
// throw(UPNPLIB_LOGEXCEPT + "MSG1nnn: exception message.\n");
#define UPNPLIB_LOGEXCEPT "UPnPlib ["+::std::string(__PRETTY_FUNCTION__)+"] EXCEPTION "

#ifdef __APPLE__
#define UPNPLIB_LOG std::cout.flush()&&std::clog<<"UPnPlib ["<<__PRETTY_FUNCTION__
#else
#define UPNPLIB_LOG std::cout.flush()&&std::osyncstream(std::clog)<<"UPnPlib ["<<__PRETTY_FUNCTION__
#endif

// Critical messages are always output.
#define UPNPLIB_LOGCRIT UPNPLIB_LOG<<"] CRITICAL "
#define UPNPLIB_LOGERR if(upnplib::g_dbug) UPNPLIB_LOG<<"] ERROR "
#define UPNPLIB_LOGCATCH if(upnplib::g_dbug) UPNPLIB_LOG<<"] CATCH "
#define UPNPLIB_LOGINFO if(upnplib::g_dbug) UPNPLIB_LOG<<"] INFO "

// clang-format on
/// \endcond

#endif // UPNPLIB_SYNCLOG_HPP
