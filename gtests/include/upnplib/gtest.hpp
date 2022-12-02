#ifndef UPNPLIB_GTEST_HPP
#define UPNPLIB_GTEST_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

#include "upnplib/visibility.hpp"
#include <cstring>
#include <regex>
#include "gmock/gmock.h"

// ANSI console colors
#define CRED "\033[38;5;203m" // red
#define CYEL "\033[38;5;227m" // yellow
#define CGRN "\033[38;5;83m"  // green
#define CRES "\033[0m"        // reset

//
namespace upnplib::testing {

//###############################
//           Helper             #
//###############################

// Capture output to stdout or stderr
// ----------------------------------
// class CaptureStdOutErr declaration
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
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
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

class UPNPLIB_API CaptureStdOutErr {
  public:
    CaptureStdOutErr(int a_fileno);
    virtual ~CaptureStdOutErr();
    void start();
    std::string get();

  private:
    int out_pipe[2]{};
    static constexpr int m_pipebuffer{8192};
    static constexpr int m_chunk_size{512};

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

//
// function to get the modification time of a file
// -----------------------------------------------
//     using ::upnplib::testing::file_mod_time;
UPNPLIB_API time_t file_mod_time(const std::string& a_pathname);

//
// function to test if file descriptors are closed
// -----------------------------------------------
//     using ::upnplib::testing::check_closed_fds;
UPNPLIB_API void check_closed_fds(int a_from_fd, int a_to_fd);

//
//###############################
//       Custom Matcher         #
//###############################

// Matcher to use portable Regex from the C++ standard library
//------------------------------------------------------------
// This overcomes the mixed internal MatchesRegex() and ContainsRegex() from
// Googlemock. On Unix it uses Posix regex but on MS Windows it uses a limited
// gmock custom implementation. If using that to be portable we are limited to
// the gmock cripple regex for MS Windows.
// Reference: https://github.com/google/googletest/issues/1208
//
// ECMAScript syntax: https://cplusplus.com/reference/regex/ECMAScript/
//
/* Example:
    using ::upnplib::testing::ContainsStdRegex;

    EXPECT_THAT(capturedStderr,
                ContainsStdRegex(" UPNP-MSER-1: .* invalid socket\\(-1\\) "));
*/
MATCHER_P(MatchesStdRegex, pattern, "") {
    std::regex regex(pattern);
    return std::regex_match(arg, regex);
}
MATCHER_P(ContainsStdRegex, pattern, "") {
    std::regex regex(pattern);
    return std::regex_search(arg, regex);
}

//
//###############################
//       Custom Actions         #
//###############################

// Action for side effect to place a value at a location
// -----------------------------------------------------
// Generate function to set value refered to by n-th argument getsockopt(). This
// allows us to mock functions that pass in a pointer, expecting the result to
// be put into that location.
// Simple version:
// ACTION_P(SetArg3PtrIntValue, value) { *static_cast<int*>(arg3) = value; }
//
/* Example:
    using ::upnplib::testing::SetArgPtrIntValue

    EXPECT_CALL(mock_sys_socketObj, getsockopt(sockfd, _, _, _, _))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(1), Return(0)));
*/
ACTION_TEMPLATE(SetArgPtrIntValue, HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(value)) {
    *static_cast<int*>(std::get<k>(args)) = value;
}

//
// Action to return a string literal
// ---------------------------------
// Reference: https://groups.google.com/g/googlemock/c/lQqCMW1ANQA
// simple version: ACTION_P(StrCpyToArg0, str) { strcpy(arg0, str); }
//
/* Example:
    using ::upnplib::testing::StrCpyToArg

    EXPECT_CALL( mocked_sys_socketObj,
        recvfrom(sockfd, _, _, _, _, _))
        .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"), Return(8)));
*/
ACTION_TEMPLATE(StrCpyToArg, HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(str)) {
    std::strcpy(std::get<k>(args), str);
}

} // namespace upnplib::testing

#endif // UPNPLIB_GTEST_HPP
