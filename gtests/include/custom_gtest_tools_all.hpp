// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-16

#ifndef UPNP_GTEST_TOOLS_H
#define UPNP_GTEST_TOOLS_H

#include "UpnpGlobal.h" /* for UPNP_INLINE, EXPORT_SPEC */
#include <string>

namespace upnp {

EXPORT_SPEC const char* UpnpGetErrorMessage(int rc);

//
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
    std::string capturedStderr;
    ASSERT_TRUE(captureObj.start());
    std::cerr << "Hello World"; // or any other output from within functions
    ASSERT_TRUE(captureObj.get(capturedStderr));
    EXPECT_THAT( capturedStderr, MatchesRegex("Hello .*"));
*/
// clang-format on

// Increase the size of the capture buffer if you get an error message.
// It is used to read the captured output from the pipe.
#define UPNP_PIPE_BUFFER_SIZE 256

class CCaptureStdOutErr {
    int m_out_pipe[2]{};
    int m_std_fileno{};
    int m_saved_stdno{};
    bool m_error = false;

  public:
    EXPORT_SPEC CCaptureStdOutErr(int);
    EXPORT_SPEC bool start(void);
    EXPORT_SPEC bool get(std::string&);
};

} // namespace upnp

#endif // UPNP_GTEST_TOOLS_H
