// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10.10

#include "upnpmock/pthreadif.hpp"
#include "upnpmock/stdioif.hpp"
#include "upnpmock/stringif.hpp"

#include "tools.h"
#include "gmock/gmock.h"

#include "api/upnpdebug.cpp"

using ::testing::_;
using ::testing::Return;

class Mock_string : public Istring {
    // Class to mock the free system functions.
  public:
    virtual ~Mock_string() {}
    Mock_string() { stringif = this; }
    MOCK_METHOD(char*, strerror, (int errnum), (override));
};

class Mock_stdio : public Istdio {
    // Class to mock the free system functions.
  public:
    virtual ~Mock_stdio() {}
    Mock_stdio() { stdioif = this; }
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode),
                (override));
    MOCK_METHOD(int, fclose, (FILE * stream), (override));
};

class Mock_pthread : public Ipthread {
    // Class to mock the free system functions.
  public:
    virtual ~Mock_pthread() {}
    Mock_pthread() { pthreadif = this; }
    MOCK_METHOD(int, pthread_mutex_init,
                (pthread_mutex_t * mutex, const pthread_mutexattr_t* mutexattr),
                (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex),
                (override));
};

// Test class for the debugging and logging module
//------------------------------------------------
class UpnpdebugMockTestSuite : public ::testing::Test {
  protected:
    // Member variables: instantiate the mock objects.
    Mock_pthread mocked_pthread;
    Mock_stdio mocked_stdio;
    Mock_string mocked_string;

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
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(setlogwascalled, 0);
    EXPECT_EQ(is_stderr, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    // Process unit again
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(setlogwascalled, 0);
    EXPECT_EQ(is_stderr, 0);
}

TEST_F(UpnpdebugMockTestSuite, set_log_level) {
    // Process unit
    UpnpSetLogLevel(UPNP_INFO);
    EXPECT_EQ(g_log_level, UPNP_INFO);
    EXPECT_EQ(setlogwascalled, 1);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_NE(is_stderr, 0);
}

TEST_F(UpnpdebugMockTestSuite, getDebugFile) {

    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)NULL);
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_but_not_to_fIle) {
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);

    // Just set the log level but no filename. This should log to stderr.
    UpnpSetLogLevel(UPNP_INFO);
    EXPECT_NE(setlogwascalled, 0);
    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_NE(is_stderr, 0);
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), stderr);
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_to_file) {
    // Set the filename, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_NE(setlogwascalled, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(is_stderr, 0);
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)0x123456abcdef);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x5a5a5a5a5a5a));

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_NE(setlogwascalled, 0);
    EXPECT_EQ(is_stderr, 0);
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)0x5a5a5a5a5a5a);
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_opening_file_fails) {
    // Set the filename, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_NE(setlogwascalled, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).WillOnce(Return((FILE*)NULL));
    // Cast from const char* to char* is possible because the string it isn't
    // changed in this scope. Otherwise it would give a runtime error.
    EXPECT_CALL(mocked_string, strerror(_))
        .WillOnce(Return((char*)"mocked error"));
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
#ifdef OLD_TEST
    std::cout << "  BUG! UpnpInitLog() should return with failure.\n";
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
#else
    EXPECT_STRNE(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
#endif
    // Will be set if failed to log to a file
    EXPECT_NE(is_stderr, 0);
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_using_file) {
    // This should enable logging
    UpnpSetLogLevel(UPNP_ALL);
    EXPECT_NE(setlogwascalled, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should log to stderr
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_NE(is_stderr, 0);

    // Now we set the filename, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_string, strerror(_)).Times(0);

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Don't log to stderr
    EXPECT_EQ(is_stderr, 0);
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_to_file_with_wrong_filename) {
    // This should enable logging
    UpnpSetLogLevel(UPNP_ALL);
    EXPECT_NE(setlogwascalled, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should log to stderr
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Log to stderr
    EXPECT_NE(is_stderr, 0);

    // Now we set a wrong filename, second parameter is unused but defined
    UpnpSetLogFileNames("", nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_string, strerror(_)).Times(0);

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Log to stderr
#ifdef OLD_TEST
    std::cout << "  BUG! With wrong filename it should log to stderr.\n";
    EXPECT_EQ(is_stderr, 0);
#else
    EXPECT_NE(is_stderr, 0);
#endif
}

TEST_F(UpnpdebugMockTestSuite, close_log) {
    // Close log without init log
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(0);
    // Process unit
    UpnpCloseLog();

    // Initialize logging
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process Init should do nothing because log not set
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Logging is initialized but not enabled
    EXPECT_EQ(setlogwascalled, 0);
    EXPECT_EQ(is_stderr, 0);

    // Close log with init log
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    // Process unit
    UpnpCloseLog();
    EXPECT_EQ(setlogwascalled, 0);
    EXPECT_EQ(is_stderr, 0);
}

TEST_F(UpnpdebugMockTestSuite, upnp_close_log_to_file) {
    // Set the filename, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_NE(setlogwascalled, 0);

    // Initialize logging to file
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));
    // Process Init should not set stderr
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(is_stderr, 0);

    // Close log with open file
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    // Process unit
    UpnpCloseLog();
    EXPECT_EQ(initwascalled, 0);
#ifdef OLD_TEST
    EXPECT_NE(setlogwascalled, 0);
    std::cout << "  BUG! UpnpCloseLog should always also disable logging, not "
                 "only Initialization.\n";
#else
    EXPECT_EQ(setlogwascalled, 0);
#endif
}

TEST_F(UpnpdebugMockTestSuite, UpnpPrintf_log_to_stderr) {
    // This should enable logging
    UpnpSetLogLevel(UPNP_ALL);
    EXPECT_NE(setlogwascalled, 0);

    // Initialize logging
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_NE(is_stderr, 0);

    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    // Process unit
    UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
               "Unit Test for %s with 'is_stderr' = %d.\n", "UpnpPrintf",
               is_stderr);
}
/*
TEST_F(UpnpdebugTestSuite, UpnpPrintf_not_initialized) {
    initwascalled = 0;

    // generate random temporary filename
    std::srand(std::time(nullptr));
    std::string fname = std::filesystem::temp_directory_path().string() +
                        "/gtest" + std::to_string(std::rand());
    fp = fopen(fname.c_str(), "a");

    // process the unit that will write to the open fp
    UpnpPrintf(UPNP_INFO, API, "gtest_filename.dummy", 0,
               "UpnpInit2 with IfName=%s, DestPort=%d.\n", "NULL", 51515);
    fclose(fp);

    // look if the output is as expected
    std::ifstream file(fname);
    std::string str;
    std::getline(file, str);
    std::remove(fname.c_str());

    // nothing happend
    EXPECT_EQ(str, "");
}
*/
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
