#ifndef UPNPLIB_SYNCLOG_HPP
#define UPNPLIB_SYNCLOG_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-04
/*!
 * \file
 * \brief Define macro for synced logging to the console for detailed info and
 * debug.
 */

#include <cmake_vars.hpp>
#include <upnplib/visibility.hpp>
/// \cond
#include <string>
#include <iostream>
#if !defined(_MSC_VER) && !defined(__APPLE__)
#include <syncstream>
#endif

namespace upnplib {
UPNPLIB_EXTERN bool g_dbug;
} // namespace upnplib


// clang-format off

// Usage: SYNC(std::cout) << "Message\n";
// Usage: SYNC(std::cerr) << "Error\n";
#if defined(_MSC_VER) || defined(__APPLE__)
  #define SYNC(s) (s)
#else
  #define SYNC(s) std::osyncstream((s))
#endif


// Trace messages
// --------------
#ifdef UPNPLIB_WITH_TRACE
  #define TRACE(s) SYNC(std::cerr)<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(s)<<"\n";
  #define TRACE2(a, b) SYNC(std::cerr)<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(a)<<(b)<<"\n";
#else // no UPNPLIB_WITH_TRACE
  #define TRACE(s)
  #define TRACE2(a, b)
#endif


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

#define UPNPLIB_LOG SYNC(std::cerr)<<"UPnPlib ["<<__PRETTY_FUNCTION__
// Critical messages are always output.
#define UPNPLIB_LOGCRIT UPNPLIB_LOG<<"] CRITICAL "
#define UPNPLIB_LOGERR if(upnplib::g_dbug) UPNPLIB_LOG<<"] ERROR "
#define UPNPLIB_LOGCATCH if(upnplib::g_dbug) UPNPLIB_LOG<<"] CATCH "
#define UPNPLIB_LOGINFO if(upnplib::g_dbug) UPNPLIB_LOG<<"] INFO "

// clang-format on
/// \endcond

#endif // UPNPLIB_SYNCLOG_HPP
