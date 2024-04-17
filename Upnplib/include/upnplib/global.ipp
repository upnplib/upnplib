// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-04-17

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

/*!
 * \file
 * \brief Global used constants, variables, functions and macros.
 */

#include <upnplib/cmake_vars.hpp>
#include <upnplib/visibility.hpp>
/// \cond
#include <string>
#include <iostream>
#ifndef __APPLE__
#include <syncstream>
#endif


// strndup() is a GNU extension.
// -----------------------------
#ifndef HAVE_STRNDUP
UPNPLIB_API char* strndup(const char* __string, size_t __n);
#endif

// clang-format off

// Trace messages
// --------------
// This compiles tracing into the source code. Once compiled in with std::clog
// (I currently use std::cout) to output you could disable TRACE with
// std::clog.setstate(std::ios_base::failbit);
// and enable with
// std::clog.clear();
// But it is not really an option because it also modifies the users program
// clog output.

#ifdef UPNPLIB_WITH_TRACE
  #define TRACE(s) std::cout<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(s)<<"\n";
  #define TRACE2(a, b) std::cout<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(a)<<(b)<<"\n";
#else
  #define TRACE(s)
  #define TRACE2(a, b)
#endif


// Debug output messages with some that can be enabled during runtime.
// -------------------------------------------------------------------
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
// This is for future expansion and not to loose information.
// Critical messages are always output.
#define UPNPLIB_LOGCRIT UPNPLIB_LOG<<"] CRITICAL "
#define UPNPLIB_LOGERR if(upnplib::g_dbug) UPNPLIB_LOG<<"] ERROR "
#define UPNPLIB_LOGCATCH if(upnplib::g_dbug) UPNPLIB_LOG<<"] CATCH "
#define UPNPLIB_LOGINFO if(upnplib::g_dbug) UPNPLIB_LOG<<"] INFO "


namespace upnplib {

// Global constants
// ================
// Default response timeout for UPnP messages as given by The UPnP™ Device
// Architecture 2.0, Document Revision Date: April 17, 2020.
inline constexpr int g_response_timeout{30};

// Info message about the library
inline constexpr std::string_view libinfo{
    "upnplib library version = under developement"};

} // namespace upnplib

// clang-format on
/// \endcond
