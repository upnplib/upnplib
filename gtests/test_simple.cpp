// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-21

#include "pthread.h" // To find pthreads4w don't use <pthread.h>
#include "gmock/gmock.h"
#include "upnplib/upnptools.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h> // must be after <winsock2.h>
#else
#include <sys/socket.h>
#endif

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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sockfd, -1);
    EXPECT_EQ(closesocket(sockfd), 0);
}

// Test library iphlpapi (iphlpapi.h)
TEST(simpleTestSuite, simple_iphlpapi_test) {
    // Just ask for a network adapter index
    wchar_t AdapterName[] = L"not_existing";
    PULONG IfIndex{0};
    EXPECT_EQ(GetAdapterIndex(AdapterName, IfIndex), 87);
}
#else

// Test unix winsock
TEST(simpleTestSuite, simple_winsock_test) {
    // Just create and close a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sockfd, -1);
    EXPECT_EQ(close(sockfd), 0);
}
#endif

TEST(simpleTestSuite, simple_upnplib_native_test) {
    EXPECT_EQ(errStr(0), "UPNP_E_SUCCESS(0)");
}

} // namespace upnplib

// main entry
// ----------
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
