// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-22

#include <upnplib/port.hpp>
#include <upnplib/general.hpp>
#include <utest/utest.hpp>


namespace utest {
bool old_code{true}; // Managed in gtest_main.inc
// bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::upnplib::g_dbug;

using ::testing::HasSubstr;


TEST(GeneralToolsTestSuite, debug_messages_successful) {
    const std::string pretty_function{"UPnPlib [" +
                                      std::string(__PRETTY_FUNCTION__) + "] "};

    // Capture output to stderr and stdout
    CaptureStdOutErr captureErrObj(STDERR_FILENO); // or STDOUT_FILENO
    CaptureStdOutErr captureOutObj(STDOUT_FILENO);
    auto dbug_old{g_dbug};

    // TRACE is a compiled in output stream with no dependency to the g_dbug
    // flag.
    captureErrObj.start();
    captureOutObj.start();
    g_dbug = false;
    TRACE("This is a TRACE output.")
    g_dbug = dbug_old;

    // There is no output to stderr.
    EXPECT_EQ(captureErrObj.str(), "");

    // There is output to stdout if TRACE is compiled in.
#ifdef UPNPLIB_WITH_TRACE
    EXPECT_THAT(captureOutObj.str(),
                MatchesStdRegex("TRACE\\[utest[/|\\\\]upnplib[/"
                                "|\\\\]test_general\\.cpp:\\d+\\] This "
                                "is a TRACE output\\.\n"));
#else
    EXPECT_EQ(captureOutObj.str(), "");
#endif

    // All UPNPLIB_LOG* messages are output to stderr.
    //
    // UPNPLIB_LOGEXCEPT is a std::string with no dependency to the g_dbug
    // flag. It is intended to be used as 'throw' message. It may be catched
    // and output there depending on the g_dbug flag.
    g_dbug = false;
    EXPECT_EQ(UPNPLIB_LOGEXCEPT + "MSG1022: this is an exception message.\n",
              pretty_function +
                  "EXCEPTION MSG1022: this is an exception message.\n");

    // UPNPLIB_LOGCRIT is an output stream with no dependency to g_dbug flag.
    g_dbug = false;
    captureErrObj.start();
    UPNPLIB_LOGCRIT << "MSG1023: this is a critical message.\n";
    EXPECT_EQ(captureErrObj.str(),
              pretty_function +
                  "CRITICAL MSG1023: this is a critical message.\n");

    // UPNPLIB_LOGERR is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureErrObj.start();
    UPNPLIB_LOGERR << "MSG1024: this is an error message.\n";
    EXPECT_EQ(captureErrObj.str(),
              pretty_function + "ERROR MSG1024: this is an error message.\n");

    g_dbug = false;
    captureErrObj.start();
    UPNPLIB_LOGERR << "MSG1025: this error message should not output.\n";
    EXPECT_EQ(captureErrObj.str(), "");

    // UPNPLIB_LOGCATCH is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureErrObj.start();
    UPNPLIB_LOGCATCH << "MSG1026: this is a catched message.\n";
    EXPECT_EQ(captureErrObj.str(),
              pretty_function + "CATCH MSG1026: this is a catched message.\n");

    g_dbug = false;
    captureErrObj.start();
    UPNPLIB_LOGCATCH << "MSG1027: this catched message should not output.\n";
    EXPECT_EQ(captureErrObj.str(), "");

    // UPNPLIB_LOGINFO is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureErrObj.start();
    UPNPLIB_LOGINFO << "MSG1028: this is an info message.\n";
    EXPECT_EQ(captureErrObj.str(),
              pretty_function + "INFO MSG1028: this is an info message.\n");

    g_dbug = false;
    captureErrObj.start();
    UPNPLIB_LOGINFO << "MSG1029: this info message should not output.\n";
    EXPECT_EQ(captureErrObj.str(), "");
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <gtest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
