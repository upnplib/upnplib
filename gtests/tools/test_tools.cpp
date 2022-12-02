// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

#include "upnplib/port.hpp"
#include "upnplib/sock.hpp"

#include "upnplib/gtest.hpp"
#include "gtest/gtest.h"

using ::testing::ThrowsMessage;

using ::upnplib::testing::CaptureStdOutErr;
using ::upnplib::testing::check_closed_fds;
using ::upnplib::testing::file_mod_time;
using ::upnplib::testing::MatchesStdRegex;

namespace upnplib {

TEST(CaptureStdOutErrTestSuite, capture_stderr_message) {
    CaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    std::cerr << "Testing error message\n";

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, "Testing error message\n");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_message) {
    CaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    std::cout << "Testing error message\n";

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, "Testing error message\n");
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_empty) {
    CaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();
    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, "");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_empty) {
    CaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();
    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, "");
}

std::string message512{
    "Begin.........................................................64"
    ".............................................................128"
    ".............................................................192"
    ".............................................................256"
    ".............................................................320"
    ".............................................................384"
    ".............................................................448"
    ".............................................................512"};

TEST(CaptureStdOutErrTestSuite, capture_stderr_with_one_chunk) {
    CaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    // Makes the message 512 bytes long like a chunk. stderr is unbufferd so we
    // do not need to delimit with "\n".
    std::cerr << message512;

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, message512);
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_with_one_chunk) {
    CaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    // Makes the message 512 bytes long like a chunk.
    std::cout << message512;

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, message512);
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_with_two_chunks) {
    CaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    // Makes the message 513 bytes long like one chunk plus one byte.
    std::cerr << message512 << "+";

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, message512 + "+");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_with_two_chunks) {
    CaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    // Makes the message 513 bytes long like one chunk plus one byte.
    std::cout << message512 << "+";

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, message512 + "+");
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_with_invalid_fd) {
    EXPECT_THROW(CaptureStdOutErr captOutObj(1023), std::invalid_argument);
}

TEST(CaptureStdOutErrTestSuite, capture_output_with_pipe) {
    CaptureStdOutErr captOutObj(STDOUT_FILENO);
    CaptureStdOutErr captErrObj(STDERR_FILENO);

    captErrObj.start();
    captOutObj.start();
    std::cerr << "err: First output ";
    std::cout << "out: First output ";
    std::cerr << "to StdErr" << std::endl;
    std::cout << "to StdOut" << std::endl;
    std::string captured_err = captErrObj.get();
    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_err, "err: First output to StdErr\n");
    EXPECT_EQ(captured_out, "out: First output to StdOut\n");
    // std::cout << "First capture " << captured_err;
    // std::cout << "First capture " << captured_out;

    captErrObj.start();
    captOutObj.start();
    std::cerr << "err: Second output to StdErr" << std::endl;
    std::cout << "out: Second output to StdOut" << std::endl;
    captured_err = captErrObj.get();
    captured_out = captOutObj.get();

    EXPECT_EQ(captured_err, "err: Second output to StdErr\n");
    EXPECT_EQ(captured_out, "out: Second output to StdOut\n");
    // std::cout << "Second start " << captured_err;
    // std::cout << "Second start " << captured_out;
}

TEST(FileModTimeTestSuite, get_modification_time) {
    // Compare with the time when this test was created. It must always be
    // greater.
    EXPECT_GT(file_mod_time(__FILE__), 1660851000);
}

TEST(FileModTimeTestSuite, get_modification_time_from_empty_filename) {
    EXPECT_THROW(file_mod_time(""), std::invalid_argument);
}

TEST(FileModTimeTestSuite, get_modification_time_from_invalid_filename) {
    EXPECT_THROW(file_mod_time("unknown.file"), std::invalid_argument);
}

TEST(FileModTimeTestSuite, get_modification_time_from_nullptr) {
#ifdef __unix__
    EXPECT_THROW(file_mod_time(nullptr), std::logic_error);
#else
    // This expects segfault from the std::string object of the argument.
    EXPECT_DEATH(file_mod_time(nullptr), ".*");
#endif
}

TEST(DISABLED_ClosedFdsTestSuite, check_closed_fds) {
    // Note: Disabled because on different environments (Github Actions, Linux,
    // macOS, MS Windows etc.) we have different numbers of open file
    // descriptors so we will not always find e valid match. Use this test only
    // if modifying check_closed_fds().
    //
    // This test excludes file descriptor 0, 1, 2 that is stdin, stdout and
    // stderr. They are always open. It also exludes fd 3 that is used by ctest
    // when running it to execute this test.
    EXPECT_NO_THROW(check_closed_fds(4, 1025));
}

TEST(DISABLED_ClosedFdsTestSuite, check_closed_fds_with_open_socket) {
    // For disabling see note at TEST(DISABLED_ClosedFdsTestSuite,
    // check_closed_fds).
    //
    // This should find an open file descriptor 5 or 6 for a socket. Besides fd
    // 0 to 2 it is possible that fd 3 is also used if executing the test with
    // ctest that uses one fd. That's the reason why we start searching with
    // fd 4.
    SOCKET sock3 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock5 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock3 != INVALID_SOCKET)
        close((int)sock3);
    if (sock4 != INVALID_SOCKET)
        close((int)sock4);

    EXPECT_THAT([]() { check_closed_fds(4, 16); },
                ThrowsMessage<std::runtime_error>(MatchesStdRegex(
                    "Found open file descriptor [56]\\. \\(0\\) Success")));

    if (sock5 != INVALID_SOCKET)
        close((int)sock5);
}

TEST(DISABLED_ClosedFdsTestSuite, check_closed_fds_with_open_file) {
    // For disabling see note at TEST(DISABLED_ClosedFdsTestSuite,
    // check_closed_fds).
    //
    // This test includes open file descriptor 2 (stderr).
    EXPECT_THAT([]() { check_closed_fds(2, 16); },
                ThrowsMessage<std::runtime_error>(
                    "Found open file descriptor 2. (88) Socket operation on "
                    "non-socket"));
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
