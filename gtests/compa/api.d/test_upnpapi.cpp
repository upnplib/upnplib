// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-26

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include <pupnp/upnp/src/api/upnpapi.cpp>
#else
#include <compa/src/api/upnpapi.cpp>
#endif

#ifdef UPNP_HAVE_TOOLS
#include <upnptools.hpp> // For pupnp and compa
#endif
#include <upnplib/upnptools.hpp> // For upnplib only

#include <upnplib/gtest.hpp>
#include <gmock/gmock.h>


namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

using ::testing::ExitedWithCode;
using ::upnplib::errStrEx;
using ::upnplib::testing::MatchesStdRegex;

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

03) TEST(UpnpapiTestSuite, UpnpInitPreamble)
04) TEST(UpnpapiTestSuite, WinsockInit)
05) Tested with ./test_upnpdebug.cpp
11) Tested with ./test_TimerThread.cpp
13) Tested with ./test_upnpapi_win32.cpp
14) Tested with ./test_upnpapi_unix.cpp
17) Tested with ./test_miniserver.cpp


01)  UpnpDownloadXmlDoc()
     |__ UpnpDownloadUrlItem()
03)  |   |__ http_Download()
     |
     |__ ixmlParseBufferEx()
     |__ free(xml_buf)
     |__ if not IXL_SUCCESS
            print critical xml error messages
        else
            #ifdef DEBUG
              print XML document
            #endif
              print success

01) A possible url is http://127.0.0.1:50001/tvdevicedesc.xml
03) Tested within test_httpreadwrite.cpp


     UpnpFinish()
     |#ifdef UPNP_ENABLE_OPEN_SSL
     |__ SSL_CTX_free()
     |#endif
     |__ if not UpnpSdkInit
            return
     |   else
     |
     |#ifdef INCLUDE_DEVICE_APIS
     |__ while GetDeviceHandleInfo()
     |      UpnpUnRegisterRootDevice()
     |#endif
     |
     |#ifdef INCLUDE_CLIENT_APIS
01)  |__ while GetClientHandleInfo()
02)  |         |__ GetHandleInfo()
     |      UpnpUnRegisterClient()
     |#endif
     |
     |__ TimerThreadShutdown()
     |__ StopMiniServer()
     |__ web_server_destroy()
     |__ ThreadPoolShutdown()
     |
     |#ifdef INCLUDE_CLIENT_APIS
     |__    ithread_mutex_destroy() for clients
     |#endif
     |
     |__ ithread_rwlock_destroy()
     |__ ithread_mutex_destroy()
     |__ UpnpRemoveAllVirtualDirs()
     |__ ithread_cleanup_library()

02) TEST(Upnpapi*, GetHandleInfo_*)

clang-format on
*/

// upnpapi TestSuites
// ==================
class UpnpapiFTestSuite : public ::testing::Test {
  protected:
    UpnpapiFTestSuite() {
        // Destroy global variables to avoid side effects.
        memset(&GlobalHndRWLock, 0xAA, sizeof(GlobalHndRWLock));
        memset(&gUUIDMutex, 0xAA, sizeof(gUUIDMutex));
        memset(&GlobalClientSubscribeMutex, 0xAA,
               sizeof(GlobalClientSubscribeMutex));
        memset(&gUpnpSdkNLSuuid, 0, sizeof(gUpnpSdkNLSuuid));
        memset(&HandleTable, 0xAA, sizeof(HandleTable));
        memset(&gSendThreadPool, 0xAA, sizeof(gSendThreadPool));
        memset(&gRecvThreadPool, 0xAA, sizeof(gRecvThreadPool));
        memset(&gMiniServerThreadPool, 0xAA, sizeof(gMiniServerThreadPool));
        memset(&gTimerThread, 0xAA, sizeof(gTimerThread));
        UpnpSdkInit = 0xAA;
    }
};
typedef UpnpapiFTestSuite UpnpEnableWebserverFTestSuite;

TEST_F(UpnpapiFTestSuite, UpnpInitPreamble) {
    // Test Unit
    // ---------
    // UpnpInitPreamble() should not use and modify the UpnpSdkInit flag.
    int ret_UpnpInitPreamble = UpnpInitPreamble();
    ASSERT_EQ(ret_UpnpInitPreamble, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitPreamble, UPNP_E_SUCCESS);

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
    // and gGenaCallback variables in miniserver.cpp. This is tested with
    // MiniServer module.

    // Check timer thread initialization
    EXPECT_EQ(gTimerThread.lastEventId, 0);
    EXPECT_EQ(gTimerThread.shutdown, 0);
    EXPECT_EQ(gTimerThread.tp, &gSendThreadPool);

    // Check if UpnpSdkInit has been modified
    EXPECT_EQ(UpnpSdkInit, 0xAA);

    // TODO
    // Check if all this is cleaned up successfully
    // --------------------------------------------
    UpnpSdkInit = 1;
    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);
}

TEST_F(UpnpapiFTestSuite, WinsockInit) {
    // This Unit only aplies to Microsoft Windows but does not do anything on
    // Unix and should always return UPNP_E_SUCCESS on Unix.
    int ret_WinsockInit = WinsockInit();
    EXPECT_EQ(ret_WinsockInit, UPNP_E_SUCCESS)
        << errStrEx(ret_WinsockInit, UPNP_E_SUCCESS);
}

TEST_F(UpnpapiFTestSuite, get_error_message) {
#ifndef UPNP_HAVE_TOOLS
    GTEST_SKIP() << "  # Option UPNPLIB_WITH_TOOLS not available.";
#else
    EXPECT_STREQ(UpnpGetErrorMessage(0), "UPNP_E_SUCCESS");
    EXPECT_STREQ(UpnpGetErrorMessage(-121), "UPNP_E_INVALID_INTERFACE");
    EXPECT_STREQ(UpnpGetErrorMessage(1), "Unknown error code");
#endif
}

TEST(UpnpapiTestSuite, GetHandleInfo_successful) {
    // Will be filled with a pointer to the requested client info.
    Handle_Info* hinfo_p{nullptr};

    // Initialize the handle list.
    for (int i = 0; i < NUM_HANDLE; ++i) {
        HandleTable[i] = nullptr;
    }
    Handle_Info hinfo0{};
    HandleTable[0] = &hinfo0;
    HandleTable[0]->HType = HND_INVALID;
    // HandleTable[1] is nullptr from initialization before;
    Handle_Info hinfo2{};
    HandleTable[2] = &hinfo2;
    HandleTable[2]->HType = HND_CLIENT;
    Handle_Info hinfo3{};
    HandleTable[3] = &hinfo3;
    HandleTable[3]->HType = HND_DEVICE;
    Handle_Info hinfo4{};
    HandleTable[4] = &hinfo4;
    HandleTable[4]->HType = HND_CLIENT;

    // Test Unit
    EXPECT_EQ(GetHandleInfo(0, &hinfo_p), HND_INVALID);
    // Out of range, nothing returned.
    EXPECT_EQ(hinfo_p, nullptr);
    EXPECT_EQ(GetHandleInfo(NUM_HANDLE, &hinfo_p), HND_INVALID);
    EXPECT_EQ(hinfo_p, nullptr);
    EXPECT_EQ(GetHandleInfo(NUM_HANDLE + 1, &hinfo_p), HND_INVALID);
    EXPECT_EQ(hinfo_p, nullptr);

    EXPECT_EQ(GetHandleInfo(1, &hinfo_p), HND_INVALID); // HandleTable nullptr
    // Nothing returned.
    EXPECT_EQ(hinfo_p, nullptr);

    EXPECT_EQ(GetHandleInfo(3, &hinfo_p), HND_DEVICE);
    // Pointer to handle info is returned.
    EXPECT_EQ(hinfo_p, &hinfo3);

    EXPECT_EQ(GetHandleInfo(4, &hinfo_p), HND_CLIENT);
    // Pointer to handle info is returned.
    EXPECT_EQ(hinfo_p, &hinfo4);
}

TEST(UpnpapiDeathTest, GetHandleInfo_with_nullptr_to_result) {
    // Provide a valid entry in the HandleTable.
    Handle_Info hinfo1{};
    hinfo1.HType = HND_CLIENT;
    HandleTable[1] = &hinfo1;

    std::cout
        << CRED "[ BUG      ] " CRES << __LINE__
        << ": nullptr argument for the result to return must not segfault.\n";

    if (old_code) {
#if defined __APPLE__ && !DEBUG
// Curiosouly this does not fail. I don't know why. Maybe it needs DEBUG mode on
// Apple to detect nullptr segfault? --Ingo
#else
        // This expects segfault.
        EXPECT_DEATH(GetHandleInfo(1, nullptr), ".*");
#endif

    } else if (!github_actions) {

        EXPECT_EQ(GetHandleInfo(1, nullptr), HND_INVALID);
    }
}

TEST_F(UpnpapiFTestSuite, UpnpFinish_successful) {
    GTEST_SKIP() << "Work in progress, test must be completed.";

    // Initialize the handle list.
    for (int i = 0; i < NUM_HANDLE; ++i)
        HandleTable[i] = nullptr;

    UpnpSdkInit = 1;

    // Test Unit
    int ret_UpnpFinish{UPNP_E_INTERNAL_ERROR};
    ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);
}

TEST_F(UpnpapiFTestSuite, UpnpFinish_without_initialization) {
    UpnpSdkInit = 0;

    // Test Unit
    int ret_UpnpFinish{UPNP_E_INTERNAL_ERROR};
    ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_FINISH)
        << errStrEx(ret_UpnpFinish, UPNP_E_FINISH);

    EXPECT_EQ(UpnpSdkInit, 0);
}

TEST_F(UpnpEnableWebserverFTestSuite, enable_and_disable) {
    UpnpSdkInit = 1;
    bWebServerState = WEB_SERVER_DISABLED;

    // Test Unit enable
    int ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_ENABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_SUCCESS);

    EXPECT_EQ(bWebServerState, WEB_SERVER_ENABLED);

    // Test Unit disable
    ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_DISABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_SUCCESS);

    EXPECT_EQ(bWebServerState, WEB_SERVER_DISABLED);
}

TEST_F(UpnpEnableWebserverFTestSuite, sdk_not_initialized) {
    UpnpSdkInit = 0;
    bWebServerState = WEB_SERVER_DISABLED;

    // Test Unit
    int ret_UpnpEnableWebserver = UpnpEnableWebserver(WEB_SERVER_ENABLED);
    EXPECT_EQ(ret_UpnpEnableWebserver, UPNP_E_FINISH)
        << errStrEx(ret_UpnpEnableWebserver, UPNP_E_FINISH);

    EXPECT_EQ(bWebServerState, WEB_SERVER_DISABLED);
}

TEST(UpnpDownloadXmlDocTestSuite, download_successful) {
    // A possible url is http://127.0.0.1:50001/tvdevicedesc.xml
    if (github_actions)
        GTEST_SKIP() << "Still needs to be done.";
    else
        GTEST_FAIL() << "Still needs to be done.";
}

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
