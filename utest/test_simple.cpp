// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-06

// This tests only upnplib code without using pupnp code or compa code. The only
// used 'upnplib' library does not provide it. So these tests belong to
// namespace upnplib.

#include <upnplib/global.hpp>
#include <upnplib/messages.hpp>
#include <upnplib/upnptools.hpp>
#include <upnplib/sockaddr.hpp>

#include "pthread.h" // To find pthreads4w don't use <pthread.h>
#include <gmock/gmock.h>

namespace upnplib {

// simple mocked class
// -------------------
class Foo {
    virtual int GetSize() const = 0;
};

class MockFoo : public Foo {
  public:
    MOCK_METHOD(int, GetSize, (), (const, override));
};

// simple testsuite
//-----------------
TEST(simpleTestSuite, simple_mock_test) {
    MockFoo mockedFoo;

    EXPECT_CALL(mockedFoo, GetSize()).Times(1);
    EXPECT_EQ(mockedFoo.GetSize(), 0);
}

void* pthread_start_routine(void*) { return nullptr; }

TEST(simpleTestSuite, simple_pthreads_test) {
    pthread_t thread;
    void* retval;

    EXPECT_EQ(pthread_create(&thread, NULL, &pthread_start_routine, NULL), 0);
    EXPECT_EQ(pthread_join(thread, &retval), 0);
    EXPECT_EQ(retval, (void*)NULL);
}

#ifdef _WIN32
// Test library ws2_32 (winsock2.h)
TEST(simpleTestSuite, simple_winsock_test) {
    // Initialize winsocket
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    EXPECT_EQ(iResult, 0);

    // Just create and close a socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sockfd, (SOCKET)-1);
    EXPECT_EQ(closesocket(sockfd), 0);

    WSACleanup();
}

// Test library iphlpapi (iphlpapi.h)
TEST(simpleTestSuite, simple_iphlpapi_test) {
    // Just ask for a network adapter index
    wchar_t AdapterName[] = L"not_existing";
    PULONG IfIndex{0};
    EXPECT_EQ(GetAdapterIndex(AdapterName, IfIndex), (DWORD)87);
}
#else

// Test unix winsock
TEST(simpleTestSuite, simple_winsock_test) {
    // Just create and close a socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sockfd, (SOCKET)-1);
    EXPECT_EQ(close(sockfd), 0);
}
#endif

TEST(simpleTestSuite, version_of_upnplib_library) {
    EXPECT_EQ(libinfo, "upnplib library version = under developement");
}

TEST(simpleTestSuite, simple_upnplib_test) {
    EXPECT_EQ(errStr(0), "UPNP_E_SUCCESS(0)");
}

} // namespace upnplib

// main entry
// ----------
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
