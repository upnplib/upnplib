// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-20

// There is no include guard '#ifndef ...' because this file shouldn't be
// included more than two times as given.

// Here we have general used functions and tools.

#include <upnplib/cmake_vars.hpp>
#include <upnplib/visibility.hpp>
#include <string>

// strndup() is a GNU extension.
// -----------------------------
#ifndef HAVE_STRNDUP
UPNPLIB_API char* strndup(const char* __string, size_t __n);
#endif

// clang-format off

// Trace messages
// --------------
// This compiles tracing into the source code. Once compiled in with std::clog
// to output you could disable TRACE with
// std::clog.setstate(std::ios_base::failbit);
// and enable with
// std::clog.clear();
// But it is not really an option because it also modifies the users program
// clog output.

#ifdef UPNPLIB_WITH_TRACE
// We first flush std::cout to have a sequential output.
  #define TRACE(s) std::cout.flush();std::clog<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(s)<<"\n";
  #define TRACE2(a, b) std::cout.flush();std::clog<<"TRACE["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "<<(a)<<(b)<<"\n";
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

#define UPNPLIB_LOG std::cout.flush()&&std::clog<<"UPnPlib ["<<__PRETTY_FUNCTION__
// This is for future expansion and not to loose information.
// Critical messages are always output.
#define UPNPLIB_LOGCRIT UPNPLIB_LOG<<"] CRITICAL "
#define UPNPLIB_LOGERR if(upnplib::g_dbug) UPNPLIB_LOG<<"] ERROR "
#define UPNPLIB_LOGCATCH if(upnplib::g_dbug) UPNPLIB_LOG<<"] CATCH "
#define UPNPLIB_LOGINFO if(upnplib::g_dbug) UPNPLIB_LOG<<"] INFO "

// clang-format on


namespace upnplib {

// Info message about the library
// ------------------------------
UPNPLIB_API std::string libinfo();

} // namespace upnplib
// vim: syntax=cpp
