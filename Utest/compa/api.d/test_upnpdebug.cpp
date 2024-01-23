// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-24

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include <Pupnp/upnp/src/api/upnpdebug.cpp>
FILE*& filed{fp}; // Other alias for variable fp
#else
#include <Compa/src/api/upnpdebug.cpp>
#endif

#include <upnplib/port.hpp>
#include <upnplib/global.hpp>
#include <upnplib/upnptools.hpp>

#include <utest/utest.hpp>
#include <umock/pthread_mock.hpp>
#include <umock/stdio_mock.hpp>

namespace utest {

using ::testing::_;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;
using ::testing::StrEq;
using ::testing::StrictMock;

using ::upnplib::errStrEx;


// Interface for the upnpdebug module
// ----------------------------------
// clang-format off
class Iupnpdebug {
  public:
    virtual ~Iupnpdebug() {}

    virtual int UpnpInitLog() = 0;
    virtual void UpnpSetLogLevel(Upnp_LogLevel log_level) = 0;
    virtual void UpnpCloseLog() = 0;
    virtual void UpnpSetLogFileNames(const char* newFileName, const char* ignored) = 0;
    // virtual void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module,
    //         const char* DbgFileName, int DbgLineNo, const char* FmtStr, ...) = 0;
    virtual FILE* UpnpGetDebugFile(Upnp_LogLevel DLevel, Dbg_Module Module) = 0;
};

class Cupnpdebug : public Iupnpdebug {
  public:
    virtual ~Cupnpdebug() override {}

    int UpnpInitLog() override {
        return ::UpnpInitLog(); }
    void UpnpSetLogLevel(Upnp_LogLevel log_level) override {
        return ::UpnpSetLogLevel(log_level); }
    void UpnpCloseLog() override {
        return ::UpnpCloseLog(); }
    void UpnpSetLogFileNames( const char* newFileName, const char* ignored) override {
        return ::UpnpSetLogFileNames(newFileName, ignored); }
    // void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module, const char* DbgFileName,
    //                 int DbgLineNo, const char* FmtStr, ...) override {
    //     return; }
    FILE* UpnpGetDebugFile( Upnp_LogLevel DLevel, Dbg_Module Module) override {
        return ::UpnpGetDebugFile(DLevel, Module); }
};
// clang-format on


// Test class for the debugging and logging module
//------------------------------------------------
class UpnpdebugFTestSuite : public ::testing::Test {
  protected:
    // constructor
    UpnpdebugFTestSuite() {
        // Clear the static variables of the unit
        g_log_level = UPNP_DEFAULT_LOG_LEVEL;
        filed = nullptr;
        setlogwascalled = 0;
        initwascalled = 0;
        fileName = nullptr;
    }
};

class UpnpdebugMockFTestSuite : public UpnpdebugFTestSuite {
  protected:
    // Member variables: instantiate the module object
    Cupnpdebug upnpdebugObj;

    // Instantiate mocking objects.
    StrictMock<umock::PthreadMock> m_pthreadObj;
    StrictMock<umock::StdioMock> m_stdioObj;
    // Inject the mocking objects into the tested code.
    umock::Pthread pthread_injectObj = umock::Pthread(&m_pthreadObj);
    umock::Stdio stdio_injectObj = umock::Stdio(&m_stdioObj);
};


// Tests for the debugging and logging module.
//--------------------------------------------
TEST_F(UpnpdebugFTestSuite, UpnpPrintf_successful) {
    // Have also a look at TEST_F(UpnpdebugMockFTestSuite, UpnpPrintf_to_stderr)

    // Enable logging
    ::UpnpSetLogLevel(UPNP_ALL);
    // ::UpnpSetLogFileNames(arg1, arg2) also enables logging. If not calling
    // then output is set to default stderr, arg2 is unused but defined for
    // compatibility.
    //
    // Initialize logging, opens output file.
    int ret_UpnpInitLog = ::UpnpInitLog();
    EXPECT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    // Check if output is set to stderr
    EXPECT_EQ(::UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
              stderr);

    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();

    // Test Unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s on line %d.\n", "UpnpPrintf", __LINE__);

    // Example: "2021-10-17 21:09:01 UPNP-API_-2: Thread:0x7F1366618740
    // [/home/ingo/devel/upnplib-dev/upnplib/utest/test_upnpdebug.cpp:535]:
    // Unit Test for UpnpPrintf on line 536.\n"
    EXPECT_THAT(captureObj.str(),
                MatchesStdRegex(
                    "\\d{4}-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d UPNP-API_-2: "
                    "Thread:0x.+ \\[.+\\]: Unit Test for UpnpPrintf on line "
                    ".+\\.\n"));

    // IMPORTANT! A single run succeeds. The test only fails after 3 repetitions
    // when not closed.
    ::UpnpCloseLog();
}

TEST_F(UpnpdebugFTestSuite, UpnpPrintf_without_init) {
    // Process unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s. It should not be called.\n", "UpnpPrintf");
    // This will enable logging but no initializing
    ::UpnpSetLogLevel(UPNP_ALL);
    // Process unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s with logging enabled but not "
                 "initialized. It should not be called.\n",
                 "UpnpPrintf");
}

TEST_F(UpnpdebugFTestSuite, close_log_and_reopen_it) {
    // Start logging with first output.
    // Enable logging
    ::UpnpSetLogLevel(UPNP_ALL);
    // Initialize logging, opens output file
    int ret_UpnpInitLog = ::UpnpInitLog();
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    ::UpnpCloseLog();

    // Only enable logging should not enable output
    ::UpnpSetLogLevel(UPNP_ALL);

    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();
    ::UpnpPrintf(
        UPNP_INFO, API, __FILE__, __LINE__,
        "This Unit Test1 \"close log and reopen it\" should not be shown.\n");

    EXPECT_EQ(captureObj.str(), "");

    ::UpnpCloseLog();

    // Only initialize logging should not enable output
    ret_UpnpInitLog = ::UpnpInitLog();
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    captureObj.start();
    ::UpnpPrintf(
        UPNP_INFO, API, __FILE__, __LINE__,
        "This Unit Test2 \"close log and reopen it\" should not be shown.\n");

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Only reenable logging after closing it must not output "
                     "messages.\n";
        EXPECT_NE(captureObj.str(), ""); // Wrong!

    } else {

        EXPECT_EQ(captureObj.str(), "");
    }

    ::UpnpCloseLog();

    // Enable logging and initialize should do.
    ::UpnpSetLogLevel(UPNP_ALL);       // enable
    ret_UpnpInitLog = ::UpnpInitLog(); // initialize
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    captureObj.start();
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test3 close log and reopen it.\n");

    EXPECT_THAT(
        captureObj.str(),
        MatchesStdRegex(
            "\\d{4}-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d UPNP-API_-2: "
            "Thread:0x.+ \\[.+\\]: Unit Test3 close log and reopen it\\.\n"));

    ::UpnpCloseLog();
}

TEST_F(UpnpdebugFTestSuite, close_log_and_only_reenable_it) {
    // Start first logging.
    ::UpnpSetLogLevel(UPNP_ALL);           // enable
    int ret_UpnpInitLog = ::UpnpInitLog(); // initialize
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    ::UpnpCloseLog();

    // Only enable logging should not enable output
    ::UpnpSetLogLevel(UPNP_ALL);

    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();
    ::UpnpPrintf(
        UPNP_INFO, API, __FILE__, __LINE__,
        "This Unit Test1 \"close log and reopen it\" should not be shown.\n");

    EXPECT_EQ(captureObj.str(), "");

    ::UpnpCloseLog();
}

TEST_F(UpnpdebugFTestSuite, close_log_and_only_reinitialize_it) {
    // Start first logging.
    ::UpnpSetLogLevel(UPNP_ALL);           // enable
    int ret_UpnpInitLog = ::UpnpInitLog(); // initialize
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    ::UpnpCloseLog();

    // Only initialize logging should not enable output
    ret_UpnpInitLog = ::UpnpInitLog();
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();
    ::UpnpPrintf(
        UPNP_INFO, API, __FILE__, __LINE__,
        "This Unit Test2 \"close log and reopen it\" should not be shown.\n");

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Only reinitialize logging after closing it must not "
                     "output messages.\n";
        EXPECT_NE(captureObj.str(), ""); // Wrong!

    } else {

        EXPECT_EQ(captureObj.str(), "");
    }

    ::UpnpCloseLog();
}

TEST_F(UpnpdebugFTestSuite, close_log_and_reenable_reinitialize_it) {
    // Start first logging.
    ::UpnpSetLogLevel(UPNP_ALL);           // enable
    int ret_UpnpInitLog = ::UpnpInitLog(); // initialize
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    ::UpnpCloseLog();

    // Enable logging and initialize should do.
    ::UpnpSetLogLevel(UPNP_ALL);       // enable
    ret_UpnpInitLog = ::UpnpInitLog(); // initialize
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test3 close log and reopen it.\n");

    EXPECT_THAT(
        captureObj.str(),
        MatchesStdRegex(
            "\\d{4}-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d UPNP-API_-2: "
            "Thread:0x.+ \\[.+\\]: Unit Test3 close log and reopen it\\.\n"));

    ::UpnpCloseLog();
}


// Tests for the debugging and logging module with mocked functions.
//------------------------------------------------------------------
TEST_F(UpnpdebugMockFTestSuite, UpnpPrintf_to_stderr) {
    // Enable logging
    ::UpnpSetLogLevel(UPNP_ALL);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);

    // Initialize logging
    ASSERT_EQ(::UpnpInitLog(), UPNP_E_SUCCESS);

    if (old_code)
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " upnpdebug should flush stdout before output debug "
                     "messages to stderr.\n";

    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(2);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(2);

    // Test Unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test debug output to stderr.\n");

    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    ::UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, initlog_but_no_log_wanted) {
    // For the pthread_mutex_t structure look at
    // https://stackoverflow.com/q/23449508/5014688

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(0);
    // Process unit again
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, set_all_log_level) {
    // Set logging for all levels
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, set_log_level_info) {
    // Process unit
    upnpdebugObj.UpnpSetLogLevel(UPNP_INFO);

    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Check if logging to stderr with enabled log_level is now enabled.
    // Setting parameter to NULL returns if a filepointer is set for any of the
    // options.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);
    // You can ask if a filepointer is set for a particular Upnp_logLevel.
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, API), nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    // It seems that option Dbg_Module is ignored. It cannot be set with
    // UpnpSetLogLevel(). This returns the same file descriptor regardless of
    // the Dbg_Module.
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, DOM), stderr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, set_log_level_error) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ERROR);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, set_log_level_critical) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, log_stderr_but_not_to_fIle) {
    EXPECT_CALL(m_stdioObj, fopen(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);

    // Just set the log level but no filename. This should log to stderr.
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);

    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Check logging by log file pointer
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, log_not_stderr_but_to_file) {
    // Set the filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_stdioObj, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x123456abcdef));

    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x123456abcdef);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(1);
    EXPECT_CALL(m_stdioObj, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x5a5a5a5a5a5a));

    // Process unit
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x5a5a5a5a5a5a);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, log_not_stderr_but_opening_file_fails) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Set the filename, second parameter is unused but defined.
    // This also enables output of messages like UpnpSetLogLevel(),
    const char filename[]{"upnpdebug.log"};
    upnpdebugObj.UpnpSetLogFileNames(filename, nullptr);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    // #ifdef _WIN32
    // Mock fopen_s for MS Windows
    //     EXPECT_CALL(m_stdioObj, fopen_s(_, StrEq(filename), "a"))
    //         .WillOnce(Return(EINVAL));
    // #else
    EXPECT_CALL(m_stdioObj, fopen(StrEq(filename), StrEq("a")))
        .WillOnce(SetErrnoAndReturn(EINVAL, (FILE*)nullptr));
    // #endif
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);

    // Test Unit
    int returned = upnpdebugObj.UpnpInitLog();

    // Also with unknown output filename given, the Unit returns successful with
    // output file set to stderr.
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, log_stderr_and_using_file) {
    // This should set logging but not enable it
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(m_stdioObj, fopen(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should enable logging and log to stderr
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // Now we set the filename, second parameter is unused but defined.
    // This set the filename but does not touch previous filepointer.
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x123456abcdef));
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);

    // Process unit. This should open a filepointer to file with set filename.
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              (FILE*)0x123456abcdef);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);

    EXPECT_CALL(m_stdioObj, fclose(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, log_stderr_and_to_file_with_wrong_filename) {
    // This should enable logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(m_stdioObj, fopen(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);

    // Test Unit
    // No filename set, this should log to stderr
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Log to stderr
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // Now we set a wrong filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("", nullptr);

    EXPECT_CALL(m_pthreadObj, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fopen(_, _)).Times(0);
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);

    // Test Unit
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Filepointer is still set to stderr, that seems to be ok so far ...
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // ... but it should not try to close stderr.
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpCloseLog() must not try to close stderr.\n";
        EXPECT_CALL(m_stdioObj, fclose(stderr)).Times(1);

    } else {

        EXPECT_CALL(m_stdioObj, fclose(stderr)).Times(0);
    }

    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockFTestSuite, close_log_without_init_log) {
    EXPECT_CALL(m_stdioObj, fclose(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_lock(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_unlock(_)).Times(0);
    EXPECT_CALL(m_pthreadObj, pthread_mutex_destroy(_)).Times(0);
    // Process unit
    upnpdebugObj.UpnpCloseLog();
}

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
