// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-10

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "tools.hpp"
#include "upnpifaddrs.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "api/upnpapi.cpp"

// UpnpApi Testsuite for IP4
//==========================
using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

// --- mock getifaddrs -------------------------------------
class MockGetifaddrs {
  public:
    MOCK_METHOD(int, getifaddrs, (struct ifaddrs**));
};

MockGetifaddrs* ptrMockGetifaddrsObj = nullptr;
int getifaddrs(struct ifaddrs** ifap) {
    return ptrMockGetifaddrsObj->getifaddrs(ifap);
}

// --- mock freeifaddrs ------------------------------------
class MockFreeifaddrs {
  public:
    MOCK_METHOD(void, freeifaddrs, (struct ifaddrs*));
};

MockFreeifaddrs* ptrMockFreeifaddrObj = nullptr;
void freeifaddrs(struct ifaddrs* ifap) {
    return ptrMockFreeifaddrObj->freeifaddrs(ifap);
}

// --- mock bind -------------------------------------------
class MockBind {
  public:
    MOCK_METHOD(int, bind, (int, const struct sockaddr*, socklen_t));
};

MockBind* ptrMockBindObj = nullptr;
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return ptrMockBindObj->bind(sockfd, addr, addrlen);
}

// --- mock if_nametoindex ---------------------------------
class MockIf_nametoindex {
  public:
    MOCK_METHOD(unsigned int, if_nametoindex, (const char*));
};

MockIf_nametoindex* ptrMockIf_nametoindexObj = nullptr;
unsigned int if_nametoindex(const char* ifname) {
    return ptrMockIf_nametoindexObj->if_nametoindex(ifname);
}

// --- mock listen -----------------------------------------
class MockListen {
  public:
    MOCK_METHOD(int, listen, (int, int));
};

MockListen* ptrMockListenObj = nullptr;
int listen(int sockfd, int backlog) {
    return ptrMockListenObj->listen(sockfd, backlog);
}

// --- mock select -----------------------------------------
class MockSelect {
  public:
    MOCK_METHOD(int, select,
                (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
                 struct timeval* timeout));
};

MockSelect* ptrMockSelectObj = nullptr;
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
           struct timeval* timeout) {
    return ptrMockSelectObj->select(nfds, readfds, writefds, exceptfds,
                                    timeout);
}

// --- mock accept -----------------------------------------
class MockAccept {
  public:
    MOCK_METHOD(int, accept,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen));
};

MockAccept* ptrMockAcceptObj = nullptr;
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return ptrMockAcceptObj->accept(sockfd, addr, addrlen);
}

// --- mock getsockname ------------------------------------
// If needed

// --- mock setsockopt--------------------------------------
class MockSetsockopt {
  public:
    MOCK_METHOD(int, setsockopt,
                (int sockfd, int level, int optname, const void* optval,
                 socklen_t optlen));
};

MockSetsockopt* ptrMockSetsockoptObj = nullptr;
int setsockopt(int sockfd, int level, int optname, const void* optval,
               socklen_t optlen) {
    return ptrMockSetsockoptObj->setsockopt(sockfd, level, optname, optval,
                                            optlen);
}

namespace upnp {

// This TestSuite is with initializing mocks
//------------------------------------------
class UpnpapiIPv4MockTestSuite : public ::testing::Test
// Fixtures for this Testsuite
{
  protected:
    unsigned short PORT = 51515;

    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    MockGetifaddrs mockGetifaddrsObj;
    MockFreeifaddrs mockFreeifaddrsObj;
    MockBind mockBindObj;
    MockIf_nametoindex mockIf_nametoindexObj;
    MockListen mockListenObj;
    MockSelect mockSelectObj;
    MockAccept mockAcceptObj;
    MockSetsockopt mockSetsockoptObj;

    // constructor of this testsuite
    UpnpapiIPv4MockTestSuite() {
        // set the global pointer to the mock objects
        ptrMockGetifaddrsObj = &mockGetifaddrsObj;
        ptrMockFreeifaddrObj = &mockFreeifaddrsObj;
        ptrMockBindObj = &mockBindObj;
        ptrMockIf_nametoindexObj = &mockIf_nametoindexObj;
        ptrMockListenObj = &mockListenObj;
        ptrMockSelectObj = &mockSelectObj;
        ptrMockAcceptObj = &mockAcceptObj;
        ptrMockSetsockoptObj = &mockSetsockoptObj;

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

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {
    // SKIP on Github Actions
    char* github_action = std::getenv("GITHUB_ACTIONS");
    if (github_action) {
        GTEST_SKIP() << "  due to issues with googlemock";
    }

    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/11");
    ifaddr = ifaddr4Obj.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_)).WillOnce(Return(2));

    // call the unit
    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpGetIfInfo("if0v4")),
                 "UPNP_E_SUCCESS");

    // gIF_NAME mocked with getifaddrs above
    EXPECT_STREQ(gIF_NAME, "if0v4");
    // gIF_IPV4 mocked with getifaddrs above
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
    // EXPECT_THAT(gIF_IPV4,
    // MatchesRegex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"));
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    // index mocked with if_nametoindex above
    EXPECT_EQ(gIF_INDEX, (const unsigned int)2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_unknown_interface) {
    // SKIP on Github Actions
    char* github_action = std::getenv("GITHUB_ACTIONS");
    if (github_action) {
        GTEST_SKIP() << "  due to issues with googlemock";
    }

    //    GTEST_SKIP() << "due to failed github sanity check because of issue
    //    #247.\n"
    //                 << "Comment GTEST_SKIP() in the TestSuite to enable this
    //                 test.";

    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("eth0", "192.168.77.48/22");
    ifaddr = ifaddr4Obj.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_)).Times(0);

    // call the unit
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpGetIfInfo("ethO")),
                 "UPNP_E_INVALID_INTERFACE");
#ifdef OLD_TEST
    std::cout
        << "  BUG! Interface name (e.g. ethO with upper case O), ip "
        << "address and netmask should not be modified on wrong entries.\n";
    // gIF_NAME mocked with getifaddrs above
    // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
    EXPECT_STREQ(gIF_NAME, "ethO");
    // gIF_IPV4 with "192.68.77.48/22" mocked by getifaddrs above
    // but get ip address from previous successful call
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
    // get netmask from previous successful call
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0");
#else
    // gIF_NAME mocked with getifaddrs above
    EXPECT_STREQ(gIF_NAME, "")
        << "ATTENTION! There is a wrong upper case 'O', not zero in \"ethO\".\n"
        << "# Interface name should not be modified on wrong entries.";
    // gIF_IPV4 mocked with getifaddrs above
    EXPECT_STREQ(gIF_IPV4, "")
        << "# Ip address should not be modified on wrong entries.";
    EXPECT_STREQ(gIF_IPV4_NETMASK, "")
        << "# Netmask should not be modified on wrong entries.";
#endif
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    // index mocked with if_nametoindex above
    EXPECT_EQ(gIF_INDEX, (unsigned)4294967295) << "    Which is: (unsigned)-1";
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, initialize_default_UpnpInit2) {
    // SKIP on Github Actions
    char* github_action = std::getenv("GITHUB_ACTIONS");
    if (github_action) {
        GTEST_SKIP()
            << "  due to failed github sanity check because of issue #272";
    }

    // provide a network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/20");
    struct ifaddrs* ifaddr = ifaddr4Obj.get();

    // expect calls to system functions (which are mocked)
    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(mockBindObj, bind(_, _, _)).Times(0);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_)).Times(1);
    EXPECT_CALL(mockListenObj, listen(_, _)).Times(0);
    EXPECT_CALL(mockSelectObj, select(_, _, _, _, _)).Times(0);
    //        .Times(AtLeast(1));
    EXPECT_CALL(mockAcceptObj, accept(_, _, _)).Times(0);
    EXPECT_CALL(mockSetsockoptObj, setsockopt(_, _, _, _, _)).Times(0);

    // Initialize capturing of the stderr output
    CCaptureFd captFdObj;
    captFdObj.capture(2); // 1 = stdout, 2 = stderr

    // call the unit
    EXPECT_EQ(UpnpSdkInit, 0);
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInit2(NULL, 0)), "UPNP_E_SUCCESS");

    EXPECT_FALSE(captFdObj.print(std::cerr))
        << "Output to stderr is true. There should not be any output to stderr";
    EXPECT_EQ(UpnpSdkInit, 1);

    // call the unit again
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInit2(NULL, 0)), "UPNP_E_INIT");
    EXPECT_EQ(UpnpSdkInit, 1);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpInitMutexes) {
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitMutexes()), "UPNP_E_SUCCESS");
}

// UpnpApi common Testsuite
//-------------------------
TEST(UpnpapiTestSuite, WinsockInit) {
    EXPECT_STREQ(UpnpGetErrorMessage(UPNP_E_SUCCESS), "UPNP_E_SUCCESS");
}

TEST(UpnpapiTestSuite, get_handle_info) {
    Handle_Info** HndInfo = 0;
    EXPECT_EQ(GetHandleInfo(0, HndInfo), HND_INVALID);
    EXPECT_EQ(GetHandleInfo(1, HndInfo), HND_INVALID);
}

TEST(UpnpapiTestSuite, get_error_message) {
    EXPECT_STREQ(UpnpGetErrorMessage(0), "UPNP_E_SUCCESS");
    EXPECT_STREQ(UpnpGetErrorMessage(-121), "UPNP_E_INVALID_INTERFACE");
    EXPECT_STREQ(UpnpGetErrorMessage(1), "Unknown error code");
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
