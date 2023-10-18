// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-20

#include <upnplib/port.hpp>
#include <upnplib/general.hpp>
#include <upnplib/gtest.hpp>
// #include <utest/utest.hpp>


namespace utest {
bool old_code{true}; // Managed in gtest_main.inc
// bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::upnplib::g_dbug;

using ::testing::HasSubstr;

using ::upnplib::testing::CaptureStdOutErr;
using ::upnplib::testing::MatchesStdRegex;


class CSave_g_debug {
  public:
    virtual ~CSave_g_debug() {
        // Restore global debug flag.
        g_dbug = m_debug_old;
    }

  private:
    // Saved old status of the global debug flag.
    const bool m_debug_old{g_dbug};
};


TEST(GeneralToolsTestSuite, debug_messages_successful) {
    const std::string pretty_function{"UPnPlib [" +
                                      std::string(__PRETTY_FUNCTION__) + "] "};

    // Save the global debug flag. It will be restored with the destructor.
    CSave_g_debug saved_g_debugObj;

    // Capture output to stderr
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO

    // TRACE is a compiled in output stream with no dependency to g_dbug
    // flag.
    g_dbug = false;
    captureObj.start();
    TRACE("This is a TRACE output.")
#ifdef UPNPLIB_WITH_TRACE
    EXPECT_THAT(captureObj.str(),
                MatchesStdRegex("TRACE\\[utest[/|\\\\]upnplib[/"
                                "|\\\\]test_general\\.cpp:\\d+\\] This "
                                "is a TRACE output\\.\n"));
#else
    EXPECT_EQ(captureObj.str(), "");
#endif

    // UPNPLIB_LOGEXCEPT is a std::string with no dependency to the g_dbug
    // flag. It is intended to be used as 'throw' message.
    g_dbug = false;
    EXPECT_EQ(UPNPLIB_LOGEXCEPT + "MSG1022: this is an exception message.\n",
              pretty_function +
                  "EXCEPTION MSG1022: this is an exception message.\n");

    // UPNPLIB_LOGCRIT is an output stream with no dependency to g_dbug flag.
    g_dbug = false;
    captureObj.start();
    UPNPLIB_LOGCRIT << "MSG1023: this is a critical message.\n";
    EXPECT_EQ(captureObj.str(),
              pretty_function +
                  "CRITICAL MSG1023: this is a critical message.\n");

    // UPNPLIB_LOGERR is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureObj.start();
    UPNPLIB_LOGERR << "MSG1024: this is an error message.\n";
    EXPECT_EQ(captureObj.str(),
              pretty_function + "ERROR MSG1024: this is an error message.\n");

    g_dbug = false;
    captureObj.start();
    UPNPLIB_LOGERR << "MSG1025: this error message should not output.\n";
    EXPECT_EQ(captureObj.str(), "");

    // UPNPLIB_LOGCATCH is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureObj.start();
    UPNPLIB_LOGCATCH << "MSG1026: this is a catched message.\n";
    EXPECT_EQ(captureObj.str(),
              pretty_function + "CATCH MSG1026: this is a catched message.\n");

    g_dbug = false;
    captureObj.start();
    UPNPLIB_LOGCATCH << "MSG1027: this catched message should not output.\n";
    EXPECT_EQ(captureObj.str(), "");

    // UPNPLIB_LOGINFO is an output stream depending on the g_dbug flag.
    g_dbug = true;
    captureObj.start();
    UPNPLIB_LOGINFO << "MSG1028: this is an info message.\n";
    EXPECT_EQ(captureObj.str(),
              pretty_function + "INFO MSG1028: this is an info message.\n");

    g_dbug = false;
    captureObj.start();
    UPNPLIB_LOGINFO << "MSG1029: this info message should not output.\n";
    EXPECT_EQ(captureObj.str(), "");
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <gtest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
