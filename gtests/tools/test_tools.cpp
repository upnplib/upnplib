// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-09

#include "upnplib/gtest.hpp"
#include "gtest/gtest.h"

using ::upnplib::testing::CCaptureStdOutErr;

namespace upnplib {

TEST(CaptureStdOutErrTestSuite, capture_stderr_message) {
    CCaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    std::cerr << "Testing error message\n";

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, "Testing error message\n");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_message) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    std::cout << "Testing error message\n";

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, "Testing error message\n");
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_empty) {
    CCaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();
    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, "");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_empty) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
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
    CCaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    // Makes the message 512 bytes long like a chunk. stderr is unbufferd so we
    // do not need to delimit with "\n".
    std::cerr << message512;

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, message512);
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_with_one_chunk) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    // Makes the message 512 bytes long like a chunk.
    std::cout << message512;

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, message512);
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_with_two_chunks) {
    CCaptureStdOutErr captErrObj(STDERR_FILENO);
    captErrObj.start();

    // Makes the message 513 bytes long like one chunk plus one byte.
    std::cerr << message512 << "+";

    std::string captured_err = captErrObj.get();

    EXPECT_EQ(captured_err, message512 + "+");
}

TEST(CaptureStdOutErrTestSuite, capture_stdout_with_two_chunks) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
    captOutObj.start();

    // Makes the message 513 bytes long like one chunk plus one byte.
    std::cout << message512 << "+";

    std::string captured_out = captOutObj.get();

    EXPECT_EQ(captured_out, message512 + "+");
}

TEST(CaptureStdOutErrTestSuite, capture_stderr_with_invalid_fd) {
    EXPECT_THROW(CCaptureStdOutErr captOutObj(1023), std::invalid_argument);
}

TEST(CaptureStdOutErrTestSuite, capture_output_with_pipe) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
    CCaptureStdOutErr captErrObj(STDERR_FILENO);

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

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
