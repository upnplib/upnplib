// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688
// Author: 2021-03-06 - Ingo Höft <Ingo@Hoeft-online.de>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "./tools/ifaddrs.cpp"

// for TestSuites needing headers linked against the static C library
extern "C" {
    #include "upnpapi.h"
    #include "upnptools.h"
}

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
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

// --- mock if_nametoindex ---------------------------------
class MockIf_nametoindex {
public:
    MOCK_METHOD(unsigned int, if_nametoindex, (const char*));
};

MockIf_nametoindex* ptrMockIf_nametoindexObj = nullptr;
unsigned int if_nametoindex(const char* ifname) {
    return ptrMockIf_nametoindexObj->if_nametoindex(ifname);
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
    MOCK_METHOD(int, select, (int nfds, fd_set* readfds, fd_set* writefds,
                              fd_set *exceptfds, struct timeval* timeout));
};

MockSelect* ptrMockSelectObj = nullptr;
int select(int nfds, fd_set* readfds, fd_set* writefds,
           fd_set* exceptfds, struct timeval* timeout) {
    return ptrMockSelectObj->select(nfds, readfds, writefds,
                                    exceptfds, timeout);
}

// --- mock accept -----------------------------------------
class MockAccept {
public:
    MOCK_METHOD(int, accept, (int sockfd, struct sockaddr* addr,
                              socklen_t* addrlen));
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
    MOCK_METHOD(int, setsockopt, (int sockfd, int level, int optname,
                                  const void* optval, socklen_t optlen));
};

MockSetsockopt* ptrMockSetsockoptObj = nullptr;
int setsockopt(int sockfd, int level, int optname, const void* optval,
               socklen_t optlen) {
    return ptrMockSetsockoptObj->setsockopt(sockfd, level, optname,
                                            optval, optlen);
}


// UpnpApi Testsuite for IP4
//==========================
class UpnpApiIPv4TestSuite: public ::testing::Test
// Fixtures for this Testsuite
{
protected:
    unsigned short PORT = 51515;

    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    MockGetifaddrs mockGetifaddrsObj;
    MockFreeifaddrs mockFreeifaddrsObj;
    MockIf_nametoindex mockIf_nametoindexObj;
    MockBind mockBindObj;
    MockListen mockListenObj;
    MockSelect mockSelectObj;
    MockAccept mockAcceptObj;
    MockSetsockopt mockSetsockoptObj;

    UpnpApiIPv4TestSuite()
    {
        #include "init_global_var.inc"

        // set the global pointer to the mock objects
        ptrMockGetifaddrsObj = &mockGetifaddrsObj;
        ptrMockFreeifaddrObj = &mockFreeifaddrsObj;
        ptrMockIf_nametoindexObj = &mockIf_nametoindexObj;
        ptrMockBindObj = &mockBindObj;
        ptrMockListenObj = &mockListenObj;
        ptrMockSelectObj = &mockSelectObj;
        ptrMockAcceptObj = &mockAcceptObj;
        ptrMockSetsockoptObj = &mockSetsockoptObj;
    }
};


TEST_F(UpnpApiIPv4TestSuite, UpnpGetIfInfo_called_with_valid_interface)
{
    struct ifaddrs* ifaddr = nullptr;

    // provide a network interface
    Ifaddr4 ifaddr4;
    ifaddr4.set("if0v4", "192.168.99.3/24");
    ifaddr = ifaddr4.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr))
        .Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_))
        .WillOnce(Return(1));

    EXPECT_STREQ(gIF_NAME, "");
    EXPECT_STREQ(gIF_IPV4, "");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_INDEX, (const unsigned int)4294967295);  // signed int -1

    EXPECT_STREQ(UpnpGetErrorMessage(
                  UpnpGetIfInfo("if0v4")), "UPNP_E_SUCCESS");

    EXPECT_STREQ(gIF_NAME, "if0v4");
    //EXPECT_THAT(gIF_IPV4, MatchesRegex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"));
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_INDEX, (const unsigned int)1);
}

TEST_F(UpnpApiIPv4TestSuite, test_global_variables)
{
    EXPECT_EQ(LOCAL_PORT_V4, 0);
    EXPECT_EQ(LOCAL_PORT_V6, 0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, 0);
    EXPECT_EQ(bWebServerState, 0);
    EXPECT_EQ(gIF_INDEX, 4294967295) << "    Which is: (unsigned)-1";
    EXPECT_STREQ(gIF_IPV4, "");
    EXPECT_STREQ(gIF_IPV4_NETMASK, "");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_NAME, "");
    EXPECT_STREQ(gUpnpSdkNLSuuid, "");
    EXPECT_EQ(g_UpnpSdkEQMaxAge, 30);
    EXPECT_EQ(g_UpnpSdkEQMaxLen, 10);
    EXPECT_EQ(g_maxContentLength, (const long unsigned int)16000);
    EXPECT_EQ(pVirtualDirList, nullptr);
}

TEST_F(UpnpApiIPv4TestSuite, UpnpGetIfInfo_called_with_unknown_interface)
{
    struct ifaddrs* ifaddr = nullptr;

    // provide a network interface
    Ifaddr4 ifaddr4;
    ifaddr4.set("eth0", "192.168.99.3/24");
    ifaddr = ifaddr4.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr))
        .Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_))
        .Times(0);

    // ATTENTION! "ethO" is a typo. There is an upper case 'O' not zero.
    EXPECT_STREQ(UpnpGetErrorMessage(
                 UpnpGetIfInfo("ethO")), "UPNP_E_INVALID_INTERFACE");

    // we expect that nothing has changed
    EXPECT_STREQ(gIF_NAME, "")
        << "ATTENTION! There is a wrong upper case 'O', not zero in \"ethO\"\n";
    EXPECT_STREQ(gIF_IPV4, "");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_INDEX, (const unsigned int)4294967295)
                          << "    Which is: (unsigned)-1";
}

TEST_F(UpnpApiIPv4TestSuite, initialize_with_UpnpInit2)
{
    struct ifaddrs* ifaddr = nullptr;

    // provide a network interface
    Ifaddr4 ifaddr4;
    ifaddr4.set("if0v4", "192.168.99.3/20");
    ifaddr = ifaddr4.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr))
        .Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_))
        .Times(1);
    EXPECT_CALL(mockBindObj, bind(_,_,_))
        .Times(5);
    EXPECT_CALL(mockListenObj, listen(_,_))
        .Times(3);
    EXPECT_CALL(mockSelectObj, select(_,_,_,_,_))
        .Times(7);
    EXPECT_CALL(mockAcceptObj, accept(_,_,_))
        .Times(3);
    EXPECT_CALL(mockSetsockoptObj, setsockopt(_,_,_,_,_))
        .Times(11);

    EXPECT_STREQ(UpnpGetErrorMessage(
                  UpnpInit2("if0v4", PORT)), "UPNP_E_SUCCESS");
}
/*
TEST_F(UpnpApiIPv4TestSuite, initialize_with_object_oUpnp)
{
    struct ifaddrs* ifaddr = nullptr;

    // provide a network interface
    Ifaddr4 ifaddr4;
    ifaddr4.set("if0v4", "192.168.99.3/20");
    ifaddr = ifaddr4.get();

    EXPECT_CALL(mockGetifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(mockFreeifaddrsObj, freeifaddrs(ifaddr))
        .Times(1);
    EXPECT_CALL(mockIf_nametoindexObj, if_nametoindex(_))
        .Times(1);
    EXPECT_CALL(mockBindObj, bind(_,_,_))
        .Times(5);
    EXPECT_CALL(mockListenObj, listen(_,_))
        .Times(3);
    EXPECT_CALL(mockSelectObj, select(_,_,_,_,_))
        .Times(7);
    EXPECT_CALL(mockAcceptObj, accept(_,_,_))
        .Times(3);
    EXPECT_CALL(mockSetsockoptObj, setsockopt(_,_,_,_,_))
        .Times(11);

    IUpnp oUpnp;
    EXPECT_STREQ(UpnpGetErrorMessage(
                  oUpnp.UpnpInit2("if0v4", PORT)), "UPNP_E_SUCCESS");
}
*/

/* first tests with IP6 not working anymore with mocking, will be improved next
// UpnpApi Testsuite for IP6
//==========================
class UpnpApiIPv6TestSuite: public ::testing::Test
{
    // Fixtures for this Testsuite
    protected:
    std::string interface = "ens1";
    unsigned short PORT = 51515;
};

TEST_F(UpnpApiIPv6TestSuite, UpnpGetIfInfo)
{
    EXPECT_EQ(UpnpGetIfInfo(interface.c_str()), UPNP_E_SUCCESS);
    EXPECT_STREQ(gIF_NAME, interface.c_str());
    EXPECT_STREQ(gIF_IPV4, "");
    //strncpy(gIF_IPV6, "fe80::5054:ff:fe40:50f6", 24); //testing the regex
    EXPECT_THAT(gIF_IPV6, MatchesRegex("([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}"));
    EXPECT_THAT(gIF_IPV6_ULA_GUA, MatchesRegex("([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}"));
    EXPECT_IN_RANGE((int)gIF_INDEX, 1, 20);
}

TEST_F(UpnpApiIPv6TestSuite, UpnpInit2)
{
    //GTEST_SKIP();
    int return_value = UpnpInit2(interface.c_str(), PORT);
    EXPECT_EQ(return_value, UPNP_E_SUCCESS);
}
*/

// UpnpApi common Testsuite
//-------------------------
TEST(UpnpApiTestSuite, get_handle_info)
{
    Handle_Info **HndInfo = 0;
    EXPECT_EQ(GetHandleInfo(0, HndInfo), HND_INVALID);
    EXPECT_EQ(GetHandleInfo(1, HndInfo), HND_INVALID);
    EXPECT_EQ(GetHandleInfo(NUM_HANDLE, HndInfo), HND_INVALID);
}

TEST(UpnpApiTestSuite, get_error_message)
{
    EXPECT_STREQ(UpnpGetErrorMessage(0), "UPNP_E_SUCCESS");
    EXPECT_STREQ(UpnpGetErrorMessage(-121), "UPNP_E_INVALID_INTERFACE");
    EXPECT_STREQ(UpnpGetErrorMessage(1), "Unknown error code");
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
