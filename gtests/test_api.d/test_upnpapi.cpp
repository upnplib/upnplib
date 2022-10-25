// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-25

#include "pupnp/upnp/src/api/upnpapi.cpp"
#ifdef UPNP_HAVE_TOOLS
#include "upnptools.hpp" // For pupnp and compa
#endif
#include "upnplib/upnptools.hpp" // For upnplib only

#include "gmock/gmock.h"
#include "upnplib/gtest.hpp"

using ::upnplib::testing::MatchesStdRegex;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// The UpnpInit2() call stack to initialize the pupnp library
//===========================================================
/*
clang-format off

     UpnpInit2()
03)  |__ ithread_mutex_lock()
03)  |__ UpnpInitPreamble()
04)  |   |__ WinsockInit() - only on _WIN32
05)  |   |__ UpnpInitLog()
     |   |__ UpnpInitMutexes()
03)  |   |__ Initialize_handle_list
03)  |   |__ UpnpInitThreadPools()
     |   |__ SetSoapCallback() - if enabled
     |   |__ SetGenaCallback() - if enabled
03)  |   |__ TimerThreadInit()
     |
     |__ UpnpGetIfInfo()
     |#ifdef _WIN32
13)  |   |__ GetAdaptersAddresses() and interface info
     |#else
14)  |   |__ getifaddrs() and interface info
     |   |__ freeifaddrs()
     |#endif
     |
     |__ UpnpInitStartServers()
17)  |   |__ StartMiniServer() - if enabled
     |   |__ UpnpEnableWebserver() - if enabled
     |       |__ if WEB_SERVER_ENABLED
     |              web_server_init()
     |           else
     |              web_server_destroy()
     |
     |__ ithread_mutex_unlock()
clang-format on

03) TEST(UpnpapiTestSuite, UpnpInitPreamble)
04) TEST(UpnpapiTestSuite, WinsockInit)
05) Tested with ./test_upnpdebug.cpp
11) Tested with ./test_TimerThread.cpp
13) Tested with ./test_upnpapi_win32.cpp
14) Tested with ./test_upnpapi_unix.cpp
17) Tested with ./test_miniserver.cpp
*/

//
// upnpapi TestSuites
// ==================
TEST(UpnpapiTestSuite, UpnpInitPreamble) {
    // Unset tested variables
    // ----------------------
    memset(&GlobalHndRWLock, 0xFF, sizeof(GlobalHndRWLock));
    memset(&gUUIDMutex, 0xFF, sizeof(gUUIDMutex));
    memset(&GlobalClientSubscribeMutex, 0xFF,
           sizeof(GlobalClientSubscribeMutex));
    memset(&gUpnpSdkNLSuuid, 0, sizeof(gUpnpSdkNLSuuid));
    memset(&HandleTable, 0xFF, sizeof(HandleTable));
    memset(&gSendThreadPool, 0xFF, sizeof(gSendThreadPool));
    memset(&gRecvThreadPool, 0xFF, sizeof(gRecvThreadPool));
    memset(&gMiniServerThreadPool, 0xFF, sizeof(gMiniServerThreadPool));
    memset(&gTimerThread, 0xFF, sizeof(gTimerThread));

    // Test Unit.
    // ----------
    int ret_UpnpInitPreamble = UpnpInitPreamble();
    ASSERT_EQ(ret_UpnpInitPreamble, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitPreamble, UPNP_E_SUCCESS);

    // Due to initialization by components the library should not have flagged
    // to be initialized. That will we do now.
    EXPECT_EQ(UpnpSdkInit, 0);
    UpnpSdkInit = 1;

    // Check initialization of debug output.
    int ret_UpnpInitLog = UpnpInitLog();
    ASSERT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    // Check if global mutexe are initialized
    EXPECT_EQ(pthread_mutex_trylock(&GlobalHndRWLock), 0);
    EXPECT_EQ(pthread_mutex_unlock(&GlobalHndRWLock), 0);

    EXPECT_EQ(pthread_mutex_trylock(&gUUIDMutex), 0);
    EXPECT_EQ(pthread_mutex_unlock(&gUUIDMutex), 0);

    EXPECT_EQ(pthread_mutex_trylock(&GlobalClientSubscribeMutex), 0);
    EXPECT_EQ(pthread_mutex_unlock(&GlobalClientSubscribeMutex), 0);

    // Check creation of a uuid
    EXPECT_THAT(gUpnpSdkNLSuuid,
                MatchesStdRegex("[[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{"
                                "4}-[[:xdigit:]]{4}-[[:xdigit:]]{12}"));

    // Check initialization of the UPnP device and client (control point) handle
    // table
    bool handleTable_initialized{true};
    for (int i = 0; i < NUM_HANDLE; ++i) {
        if (HandleTable[i] != nullptr) {
            handleTable_initialized = false;
            break;
        }
    }
    EXPECT_TRUE(handleTable_initialized);

    // Check threadpool initialization
    EXPECT_EQ(gSendThreadPool.totalThreads, 3);
    EXPECT_EQ(gSendThreadPool.busyThreads, 1);
    EXPECT_EQ(gSendThreadPool.persistentThreads, 1);

    EXPECT_EQ(gRecvThreadPool.totalThreads, 2);
    EXPECT_EQ(gRecvThreadPool.busyThreads, 0);
    EXPECT_EQ(gRecvThreadPool.persistentThreads, 0);

    EXPECT_EQ(gMiniServerThreadPool.totalThreads, 2);
    EXPECT_EQ(gMiniServerThreadPool.busyThreads, 0);
    EXPECT_EQ(gMiniServerThreadPool.persistentThreads, 0);

    // Check settings of MiniServer callback functions SetSoapCallback() and
    // SetGenaCallback() aren't possible wihout access to static gSoapCallback
    // and gGenaCallback variables in miniserver.cpp. May be tested with
    // MiniServer module.

    // Check timer thread initialization
    EXPECT_EQ(gTimerThread.lastEventId, 0);
    EXPECT_EQ(gTimerThread.shutdown, 0);
    EXPECT_EQ(gTimerThread.tp, &gSendThreadPool);

    // TODO
    // Check if all this is cleaned up successfully
    // --------------------------------------------
    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);
}

TEST(UpnpapiTestSuite, WinsockInit) {
    // This Unit only aplies to Microsoft Windows but does not do anything on
    // Unix and should always return UPNP_E_SUCCESS on Unix.
    int ret_WinsockInit = WinsockInit();
    EXPECT_EQ(ret_WinsockInit, UPNP_E_SUCCESS)
        << errStrEx(ret_WinsockInit, UPNP_E_SUCCESS);
}

TEST(UpnpapiTestSuite, get_error_message) {
#ifndef UPNP_HAVE_TOOLS
    GTEST_SKIP() << "  # Option UPNPLIB_WITH_TOOLS not available.";
#else
    EXPECT_STREQ(UpnpGetErrorMessage(0), "UPNP_E_SUCCESS");
    EXPECT_STREQ(UpnpGetErrorMessage(-121), "UPNP_E_INVALID_INTERFACE");
    EXPECT_STREQ(UpnpGetErrorMessage(1), "Unknown error code");
#endif
}

TEST(UpnpapiTestSuite, UpnpFinish_without_initialization) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        std::cout
            << "[ BUG!     ] UpnpFinish should never segfault, even without "
               "initialization.\n";

    } else {

        // Test Unit.
        UpnpSdkInit = 1;

        ASSERT_EXIT((UpnpFinish(), exit(0)), ::testing::ExitedWithCode(0), ".*")
            << "[ BUG!     ] UpnpFinish should never segfault, even "
               "without initialization.\n";
    }
}

TEST(UpnpEnableWebserverTestSuite, successful_call) {
    UpnpSdkInit = 1;
    bWebServerState = WEB_SERVER_DISABLED;

    // Test Unit
    int ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_ENABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_SUCCESS);

    EXPECT_EQ(bWebServerState, WEB_SERVER_ENABLED);
}

TEST(UpnpEnableWebserverTestSuite, sdk_not_initialized) {
    UpnpSdkInit = 0;
    bWebServerState = WEB_SERVER_DISABLED;

    // Test Unit
    int ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_ENABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_FINISH)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_FINISH);
}

TEST(UpnpEnableWebserverTestSuite, web_server_disabled) {
    UpnpSdkInit = 1;
    bWebServerState = WEB_SERVER_ENABLED;

    // Test Unit
    int ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_DISABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_SUCCESS);

    EXPECT_EQ(bWebServerState, WEB_SERVER_DISABLED);
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
}
