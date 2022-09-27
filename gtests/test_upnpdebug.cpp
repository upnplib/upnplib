// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-27

#include "pupnp/upnp/src/api/upnpdebug.cpp"

#include "upnplib/port.hpp"
#include "upnplib/upnptools.hpp"
#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;
using ::testing::StrEq;

using ::upnplib::testing::CaptureStdOutErr;
using ::upnplib::testing::MatchesStdRegex;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
// Interface for the upnpdebug module
// ----------------------------------
// clang-format off
class Iupnpdebug {
  public:
    virtual ~Iupnpdebug() {}

    virtual int UpnpInitLog(void) = 0;
    virtual void UpnpSetLogLevel(Upnp_LogLevel log_level) = 0;
    virtual void UpnpCloseLog(void) = 0;
    virtual void UpnpSetLogFileNames(const char* newFileName, const char* ignored) = 0;
    // virtual void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module,
    //         const char* DbgFileName, int DbgLineNo, const char* FmtStr, ...) = 0;
    virtual FILE* UpnpGetDebugFile(Upnp_LogLevel DLevel, Dbg_Module Module) = 0;
};

class Cupnpdebug : public Iupnpdebug {
  public:
    virtual ~Cupnpdebug() override {}

    int UpnpInitLog(void) override {
        return ::UpnpInitLog(); }
    void UpnpSetLogLevel(Upnp_LogLevel log_level) override {
        return ::UpnpSetLogLevel(log_level); }
    void UpnpCloseLog(void) override {
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

//
// Mocked system calls
// -------------------
class StdioMock : public mocking::StdioInterface {
  public:
    virtual ~StdioMock() override {}

#ifdef _WIN32
    // Secure function only on MS Windows
    MOCK_METHOD(errno_t, fopen_s,
                (FILE * *pFile, const char* pathname, const char* mode),
                (override));
#endif
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode),
                (override));
    MOCK_METHOD(int, fclose, (FILE * stream), (override));
    MOCK_METHOD(int, fflush, (FILE * stream), (override));
};

class PthreadMock : public mocking::PthreadInterface {
  public:
    virtual ~PthreadMock() override {}

    MOCK_METHOD(int, pthread_mutex_init,
                (pthread_mutex_t * mutex, const pthread_mutexattr_t* mutexattr),
                (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_cond_init,
                (pthread_cond_t * cond, pthread_condattr_t* cond_attr),
                (override));
    MOCK_METHOD(int, pthread_cond_signal, (pthread_cond_t * cond), (override));
    MOCK_METHOD(int, pthread_cond_broadcast, (pthread_cond_t * cond),
                (override));
    MOCK_METHOD(int, pthread_cond_wait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_cond_timedwait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex,
                 const struct timespec* abstime),
                (override));
    MOCK_METHOD(int, pthread_cond_destroy, (pthread_cond_t * cond), (override));
};

//
// Test class for the debugging and logging module without fixtures.
// IMPORTANT! Due to the global mocking pointer this testsuite must
// run first.
//------------------------------------------------------------------
TEST(UpnpdebugTestSuite, UpnpPrintf_without_init) {
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

TEST(UpnpdebugTestSuite, UpnpPrintf_normal_use) {
    CaptureStdOutErr captureObj(STDERR_FILENO);

    // Enable and initialize logging
    ::UpnpSetLogLevel(UPNP_ALL);

    int returned = ::UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_EQ(::UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
              stderr);

    captureObj.start();

    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s on line %d.\n", "UpnpPrintf", __LINE__);

    std::string captured = captureObj.get();

    // Example: "2021-10-17 21:09:01 UPNP-API_-2: Thread:0x7F1366618740
    // [/home/ingo/devel/upnplib-dev/upnplib/gtests/test_upnpdebug.cpp:535]:
    // Unit Test for UpnpPrintf on line 536.\n"
    EXPECT_THAT(captured,
                MatchesStdRegex(
                    "\\d{4}-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d UPNP-API_-2: "
                    "Thread:0x.+ \\[.+\\]: Unit Test for UpnpPrintf on line "
                    ".+\\.\n"));
}

//
// Test class for the debugging and logging module
//------------------------------------------------
class UpnpdebugMockTestSuite : public ::testing::Test {
  protected:
    // Member variables: instantiate the module object
    Cupnpdebug upnpdebugObj;

    // instantiate the mock objects.
    PthreadMock mocked_pthread;
    StdioMock mocked_stdio;

    // constructor
    UpnpdebugMockTestSuite() {
        // Clear the static variables of the unit
        g_log_level = UPNP_DEFAULT_LOG_LEVEL;
        fp = nullptr;
        is_stderr = 0;
        setlogwascalled = 0;
        initwascalled = 0;
        fileName = nullptr;
    }
};

TEST_F(UpnpdebugMockTestSuite, initlog_but_no_log_wanted)
// For the pthread_mutex_t structure look at
// https://stackoverflow.com/q/23449508/5014688
{
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    // Process unit again
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_all_log_level) {
    // Set logging for all levels
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
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

    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_info) {
    // Process unit
    upnpdebugObj.UpnpSetLogLevel(UPNP_INFO);

    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), nullptr);

    mocking::Pthread pthread_injectObj(&mocked_pthread);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
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

    if (old_code) {
        // It seems that option Dbg_Module is ignored. It cannot be set with
        // UpnpSetLogLevel().
        std::cout << CYEL "[ BUG      ]" CRES
                  << " Parameter Dbg_Module should not be ignored.\n";
        EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), stderr);
        EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, DOM), stderr);

    } else {

        // Only one Dbg_Module should be used.
        EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), stderr);
        EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, DOM), nullptr)
            << "  # Parameter Dbg_Module should not be ignored.";
    }

    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_error) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ERROR);
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
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

    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_critical) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
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

    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_but_not_to_fIle) {
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);

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

    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_to_file) {
    // Set the filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x123456abcdef));

    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x123456abcdef);

    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(this->mocked_stdio, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x5a5a5a5a5a5a));

    // Process unit
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x5a5a5a5a5a5a);

    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_opening_file_fails) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Set the filename, second parameter is unused but defined
    constexpr char filename[]{"upnpdebug.log"};
    upnpdebugObj.UpnpSetLogFileNames(filename, nullptr);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // #ifdef _WIN32
    // Mock fopen_s for MS Windows
    //     EXPECT_CALL(this->mocked_stdio, fopen_s(_, StrEq(filename), "a"))
    //         .WillOnce(Return(EINVAL));
    // #else
    EXPECT_CALL(this->mocked_stdio, fopen(StrEq(filename), StrEq("a")))
        .WillOnce(SetErrnoAndReturn(EINVAL, (FILE*)NULL));
    // #endif
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);

    // Process unit
    int returned = upnpdebugObj.UpnpInitLog();
    if (old_code) {
        std::cout << CYEL "[ BUG      ]" CRES
                  << " UpnpInitLog() should return with failure.\n";
        EXPECT_EQ(returned, UPNP_E_SUCCESS)
            << errStrEx(returned, UPNP_E_SUCCESS);

    } else {

        EXPECT_EQ(returned, UPNP_E_FILE_NOT_FOUND)
            << errStrEx(returned, UPNP_E_FILE_NOT_FOUND);
    }

    // Will be set to stderr if failed to log to a file
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_using_file) {
    // This should set logging but not enable it
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(this->mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);

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

    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fopen(_, StrEq("a")))
        .WillOnce(Return((FILE*)0x123456abcdef));
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);

    // Process unit. This should open a filepointer to file with set filename.
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              (FILE*)0x123456abcdef);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);

    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_to_file_with_wrong_filename) {
    // This should enable logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(this->mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should log to stderr
    int returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Log to stderr
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // Now we set a wrong filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("", nullptr);

    EXPECT_CALL(this->mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);

    // Process unit
    returned = upnpdebugObj.UpnpInitLog();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    // Filepointer is still set to stderr, that seems to be ok so far ...
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // ... but it should not try to close stderr.
    if (old_code) {
        std::cout << CYEL "[ BUG      ]" CRES
                  << " UpnpCloseLog() tries to close stderr.\n";
        EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(1);

    } else {

        std::cout << "  # UpnpCloseLog() tries to close stderr.\n";
        EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    }

    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, close_log_without_init_log) {
    mocking::Pthread pthread_injectObj(&mocked_pthread);
    mocking::Stdio stdio_injectObj(&mocked_stdio);
    EXPECT_CALL(this->mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_lock(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_unlock(_)).Times(0);
    EXPECT_CALL(this->mocked_pthread, pthread_mutex_destroy(_)).Times(0);
    // Process unit
    upnpdebugObj.UpnpCloseLog();
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
