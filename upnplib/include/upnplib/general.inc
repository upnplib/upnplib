// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-12

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


namespace upnplib {

// Debug output message that can be enabled during runtime.
// --------------------------------------------------------
#define UPNPLIB_DBUG std::cout.flush()&&std::clog<<"DEBUG["<<(static_cast<const char*>(__FILE__) + UPNPLIB_PROJECT_PATH_LENGTH)<<":"<<__LINE__<<"] "
// This is for future expansion and not to loose information
#define UPNPLIB_LOGCRT if(upnplib::g_dbug) UPNPLIB_DBUG<<"CRITICAL: "
#define UPNPLIB_LOGERR if(upnplib::g_dbug) UPNPLIB_DBUG<<"ERROR: "
#define UPNPLIB_LOGINF if(upnplib::g_dbug) UPNPLIB_DBUG<<"INFO: "
#define UPNPLIB_LOGDET if(upnplib::g_dbug) UPNPLIB_DBUG<<"DETAIL: "

// clang-format on


// Info message about the library
// ------------------------------
UPNPLIB_API std::string libinfo();

} // namespace upnplib
// vim: syntax=cpp
