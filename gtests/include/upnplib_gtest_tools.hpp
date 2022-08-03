// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-07

#ifndef UPNPLIB_GTEST_TOOLS_HPP
#define UPNPLIB_GTEST_TOOLS_HPP

#include "upnplib/port.hpp"
#include <string>

namespace upnplib {

// Capture output to stdout or stderr
// ----------------------------------
// class CCaptureStdOutErr declaration
//
// Helper class to capture output to stdout or stderr into a buffer so we can
// compare it. We use a pipe that is opened non blocking if not on Microsoft
// Windows. That does not support unblocking on its _pipe(). I workaround it
// with always writing a '\0' to the pipe just before reading it. So it will
// never block. But we risk a deadlock when the internal pipe buffer is to small
// to capture all data. Write to a full buffer will be blocked. Then we do not
// have any chance to read the pipe to free the buffer. Maybe it could be
// managed with asynchronous mode on a pipe but that is far away from Posix
// compatible handling. This deadlock is only a problem on MS Windows.
//
// If you run into a deadlock on MS Windows then increase the 'pipebuffer'.
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
//              Only stdout or stderr can be redirected.
//    throws: runtime_error
//              Failed to create a pipe.
//              Failed to duplicate a file descriptor.
//              Failed to read from pipe.
//              Read 0 byte from pipe.
// clang-format on

class UPNPLIB_API CCaptureStdOutErr {
  public:
    CCaptureStdOutErr(int a_fileno);
    virtual ~CCaptureStdOutErr();
    void start();
    std::string get();

  private:
    int out_pipe[2]{};
    static constexpr int pipebuffer{8192};
    static constexpr int chunk_size{512};

    // The original file descriptor STDOUT_FILENO or STDERR_FILENO that is
    // captured.
    int stdOutErrFd{};

    // This is the current stdout/stderr file descriptor that either points to
    // the system output or to the pipes write end.
    int current_stdOutErrFd;

    // This is a dup of the original system stdout/stderr file descriptor so we
    // can restore it to the current_stdOutErrFd.
    int orig_stdOutErrFd{};
};

} // namespace upnplib

#endif // UPNPLIB_GTEST_TOOLS_HPP
