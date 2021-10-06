// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10.06

#include "tools.h"
#include "gmock/gmock.h"

#include <stdio.h>

#include "upnpmock/pthreadif.h"
#include "upnpmock/stdioif.h"
#include "upnpmock/stringif.h"

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
    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    // process unit again
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
}

TEST_F(UpnpdebugMockTestSuite, close_log) {
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(0);
    // process unit
    UpnpCloseLog();

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    // process unit
    UpnpCloseLog();
}
/*
TEST_F(UpnpdebugMockTestSuite, setLogLevel) {
    // process unit
    UpnpSetLogLevel(UPNP_INFO);

    EXPECT_CALL(mock_pthread_mutex_init, pthread_mutex_init(_, _)).Times(1);
    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
}

TEST_F(UpnpdebugMockTestSuite, getDebugFile) {

    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)NULL);
}
*/
TEST_F(UpnpdebugMockTestSuite, log_stderr_but_not_to_fIle) {
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);

    // Just set the log level but no filename. This should log to stderr.
    UpnpSetLogLevel(UPNP_INFO);
    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), stderr);
}
/*
TEST_F(UpnpdebugMockTestSuite, logNotStderrButToFile) {
    // Set the filenmae, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);

    EXPECT_CALL(mock_pthread_mutex_init, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mock_fclose, fclose(_)).Times(0);
    EXPECT_CALL(mock_fopen, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));

    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)0x123456abcdef);

    EXPECT_CALL(mock_pthread_mutex_init, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mock_fclose, fclose(_)).Times(1);
    EXPECT_CALL(mock_fopen, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x5a5a5a5a5a5a));

    // process unit
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    // Get file pointer
    EXPECT_EQ(UpnpGetDebugFile(UPNP_INFO, API), (FILE*)0x5a5a5a5a5a5a);
}
*/
TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_opening_file_fails) {
    // Set the filenmae, second parameter is unused but defined
    UpnpSetLogFileNames("upnpdebug.log", nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, "a")).WillOnce(Return((FILE*)NULL));
    // Cast from const char* to char* is possible because the string it isn't
    // changed in this scope. Otherwise it would give a runtime error.
    EXPECT_CALL(mocked_string, strerror(_))
        .WillOnce(Return((char*)"mocked error"));
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // process unit
#ifdef OLD_TEST
    std::cout << "  BUG! UpnpInitLog() should return with failure.\n";
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
#else
    EXPECT_STRNE(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
#endif
}
/*
TEST_F(UpnpdebugMockTestSuite, logStderrAndUsingFile) {
    initwascalled = 1;
    setlogwascalled = 1;
    is_stderr = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = (char*)"gtest.log";

#ifdef OLD_TEST
    std::cout << "  BUG! fopen should not be called.\n";
    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a")).Times(1);
#else
    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a")).Times(0);
#endif
    EXPECT_CALL(mock_strerror, strerror(_))
        .Times(1)
        .WillOnce(Return((char*)"mocked error"));
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
    EXPECT_NE(fp, (FILE*)0x123456abcde0);
    EXPECT_STREQ(fileName, (char*)"gtest.log");
    EXPECT_EQ(is_stderr, 1);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, logStderrAndToFileWithWrongFilename) {
    initwascalled = 1;
    setlogwascalled = 1;
    is_stderr = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = nullptr;

    EXPECT_CALL(mock_fopen, fopen(nullptr, "a")).Times(0);
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
#ifdef OLD_TEST
    std::cout << "  BUG! fp should be set to stderr.\n";
    EXPECT_EQ(fp, (FILE*)0x123456abcde0);
    EXPECT_EQ(is_stderr, 0);
#else
    EXPECT_NE(fp, (FILE*)0x123456abcde0) << "# fp should be set to stderr.";
    EXPECT_EQ(is_stderr, 1) << "# fp should be set to stderr.";
#endif
    EXPECT_EQ(fileName, nullptr);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLog) {
    initwascalled = 1;
    pthread_mutex_init(&GlobalDebugMutex, NULL);

    // process the unit
    UpnpCloseLog();

    EXPECT_EQ(fp, nullptr);
    EXPECT_EQ(is_stderr, 0);
    EXPECT_EQ(initwascalled, 0);
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLogWithOpenLogfile) {
    pthread_mutex_init(&GlobalDebugMutex, NULL);
    initwascalled = 1;
    fp = (FILE*)0x123456abcdef;
    is_stderr = 0;
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(1);

    // process the unit
    UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLogStderrOn) {
    pthread_mutex_init(&GlobalDebugMutex, NULL);
    initwascalled = 1;
    fp = (FILE*)0x123456abcdef;
    is_stderr = 1;
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    // process the unit
    UpnpCloseLog();
}
*/
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
