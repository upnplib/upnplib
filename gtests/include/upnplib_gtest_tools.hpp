// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-09

#ifndef UPNP_GTEST_TOOLS_H
#define UPNP_GTEST_TOOLS_H

#include "upnplib/port.hpp"
#include <string>

namespace upnplib {

// Capture output to stdout or stderr
// ----------------------------------
// class CCaptureStdOutErr declaration
//
// Helper class to capture output to stdout or stderr into a buffer so we can
// compare it. Previous version with capturing to a temporary disk file can be
// found at git commit 884a040.
// clang-format off
//
// Typical usage is:
/*
    CCaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();
    std::cerr << "Hello World"; // or any other output from within functions
    std::string capturedStderr = captureObj.get();
    EXPECT_THAT(capturedStderr, MatchesRegex("Hello .*"));
*/
// Exception: Strong guarantee (no modifications)
//    throws: [std::logic_error] <- std::invalid_argument
//            Only stdout or stderr can be redirected.
//    throws: runtime_error
//            Creating a pipe failed.
// clang-format on

class UPNPLIB_API CCaptureStdOutErr {
  public:
    CCaptureStdOutErr(int a_fileno);
    virtual ~CCaptureStdOutErr();
    void start();
    std::string get();

  private:
    UPNPLIB_LOCAL static constexpr int m_chunk_size{512};
    int m_out_pipe[2]{};
    int m_std_fileno{};
    int m_saved_stdno{};
};

} // namespace upnplib

#endif // UPNP_GTEST_TOOLS_H
