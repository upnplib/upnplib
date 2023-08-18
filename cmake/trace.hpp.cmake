#ifndef UPNPLIB_CMAKE_TRACE_HPP
#define UPNPLIB_CMAKE_TRACE_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-18

// This compiles tracing into the source code. Once compiled in with std::clog
// to output you can disable TRACE with
// std::clog.setstate(std::ios_base::failbit);
// and enable with
// std::clog.clear();

// clang-format off

#ifdef UPNPLIB_WITH_TRACE
  #define TRACE(s) std::cout<<"TRACE["<<((char*)__FILE__ + ${UPNPLIB_PROJECT_PATH_LENGTH})<<":"<<__LINE__<<"] "<<(s)<<std::endl;
  #define TRACE2(a, b) std::cout<<"TRACE["<<((char*)__FILE__ + ${UPNPLIB_PROJECT_PATH_LENGTH})<<":"<<__LINE__<<"] "<<(a)<<(b)<<std::endl;
#else
  #define TRACE(s)
  #define TRACE2(a, b)
#endif

// clang-format on

#endif // UPNPLIB_CMAKE_TRACE_HPP
// vim: syntax=cpp
