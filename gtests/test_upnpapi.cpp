// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-04

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "gmock/gmock.h"

#include "api/upnpapi.cpp"

namespace upnp {

// UpnpApi Testsuite for IP4
//==========================

// This TestSuite is with initializing mocks
//------------------------------------------
class UpnpapiIPv4MockTestSuite : public ::testing::Test
// Fixtures for this Testsuite
{
  protected:
    // constructor of this testsuite
    UpnpapiIPv4MockTestSuite() {
        // initialize global variables with file scope for upnpapi.cpp
        virtualDirCallback = {};
        pVirtualDirList = nullptr;
        // GlobalClientSubscribeMutex = {}; // mutex, must be initialized,
        // only used with gena.h
        GlobalHndRWLock = {}; // mutex, must be initialzed
        // gTimerThread             // must be initialized
        gSDKInitMutex = PTHREAD_MUTEX_INITIALIZER;
        gUUIDMutex = {}; // mutex, must be initialzed
        // gSendThreadPool          // type ThreadPool must be initialized
        // gRecvThreadPool;         // type ThreadPool must be initialized
        // gMiniServerThreadPool;   // type ThreadPool must be initialized
        bWebServerState = WEB_SERVER_DISABLED;
        // Due to a bug there is annoying warning with initializing gIF_*:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99578
        // gIF_NAME[LINE_SIZE] = {'\0'};
        // gIF_IPV4[INET_ADDRSTRLEN] = {'\0'};
        // gIF_IPV4_NETMASK[INET_ADDRSTRLEN] = {'\0'};
        // gIF_IPV6[INET6_ADDRSTRLEN] = {'\0'};
        gIF_IPV6_PREFIX_LENGTH = 0;
        // gIF_IPV6_ULA_GUA[INET6_ADDRSTRLEN] = {'\0'};
        gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
        gIF_INDEX = (unsigned)-1;
        LOCAL_PORT_V4 = 0;
        LOCAL_PORT_V6 = 0;
        LOCAL_PORT_V6_ULA_GUA = 0;
        HandleTable[NUM_HANDLE] = {};
        g_maxContentLength = DEFAULT_SOAP_CONTENT_LENGTH;
        g_UpnpSdkEQMaxLen = MAX_SUBSCRIPTION_QUEUED_EVENTS;
        g_UpnpSdkEQMaxAge = MAX_SUBSCRIPTION_EVENT_AGE;
        UpnpSdkInit = 0;
        UpnpSdkClientRegistered = 0;
        UpnpSdkDeviceRegisteredV4 = 0;
        UpnpSdkDeviceregisteredV6 = 0;
#ifdef UPNP_HAVE_OPTSSDP
        gUpnpSdkNLSuuid = {};
#endif /* UPNP_HAVE_OPTSSDP */
#ifdef UPNP_ENABLE_OPEN_SSL
        SSL_CTX* gSslCtx = nullptr;
#endif
    }
};

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
