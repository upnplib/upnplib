// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-08

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include <pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp>
#else
#include <compa/src/genlib/net/http/httpreadwrite.cpp>
#endif

#include <upnplib/gtest.hpp>
#include <upnplib/sockaddr.hpp>

#include <umock/sys_socket_mock.hpp>
#include <umock/sys_select_mock.hpp>
#include <umock/pupnp_sock_mock.hpp>
#include <umock/winsock2_mock.hpp>


namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

using ::upnplib::SSockaddr_storage;

using ::upnplib::testing::SetArgPtrIntValue;


// ######################################
// Mocked system calls
// ######################################
class PupnpHttpRwMock : public umock::PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwMock() override = default;
    MOCK_METHOD(int, private_connect,
                (SOCKET sockfd, const struct sockaddr* serv_addr,
                 socklen_t addrlen),
                (override));
};


// ######################################
// testsuite for Ip4 httpreadwrite
// ######################################
#if 0
TEST(CheckConnectAndWaitConnectionIp4TestSuite, real_connect) {
    // This is for humans only to check on a Unix operating system how the Unit
    // works in realtime so we can correct mock it. Don't set '#if true'
    // permanently because it connects to the real internet and may slow down
    // this gtest dramatically, in particular with a bad or no connection. You
    // may change the ip address if google.com changed it.
    //
    // Helpful information: [Blocking vs. non-blocking sockets]
    // (https://www.scottklement.com/rpg/socktut/nonblocking.html)

    // Get a TCP socket
    int sockfd;
    ASSERT_NE(sockfd = ::socket(AF_INET, SOCK_STREAM, 0), -1)
        << ::strerror(errno);

    // Fill an address structure
    ::sockaddr_in saddrin{};
    saddrin.sin_family = AF_INET;
    saddrin.sin_port = htons(80);
    // This was a valid ip address from google.com
    saddrin.sin_addr.s_addr = ::inet_addr("172.217.18.110");

    // disable blocking of the connection, means 'connect()' will not wait until
    // the connection persists.
    ASSERT_EQ(sock_make_no_blocking(sockfd), 0);

    // Connect to the server
    int connect_returned;
    ASSERT_EQ(connect_returned = ::connect(sockfd, (const sockaddr*)&saddrin,
                                           sizeof(struct sockaddr_in)), -1);
    // 'connect()' does not wait because of no blocking mode. Instead it returns
    // immediately with the message 'Operation now in progress':
    int connect_errno = errno; // To be on the safe side
    EXPECT_EQ(connect_errno, EINPROGRESS)
        << "  # Should be EINPROGRESS(" << EINPROGRESS << ")='"
        << ::strerror(EINPROGRESS) << "' but not '" << ::strerror(connect_errno) << "'("
        << connect_errno << ").";

    // Test the Unit
    errno = connect_errno; // errno from 'connect()' is checked by the Unit
    if (old_code) {
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(sockfd, connect_returned),
                  0)
            << ::strerror(errno);
    } else {
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(sockfd,
                                                             connect_returned),
                  0)
            << ::strerror(errno);
    }

    EXPECT_EQ(::close(sockfd), 0);
}
#endif


class PrivateConnectFTestSuite : public ::testing::Test {
  protected:
    // Fictive socket file descriptor for mocking.
    const int m_sockfd{FD_SETSIZE - 40};
    // Ip address structure
    SSockaddr_storage m_saddr;

    PrivateConnectFTestSuite() { m_saddr = "[2001:db8::a]:443"; }
};


TEST_F(PrivateConnectFTestSuite, connect_successful) {
    // Steps as given by the Unit:
    // 1. sock_make_no_blocking
    // 2. try to connect
    // 3. check connection and wait until connected or timed out
    //    3.1 _WIN32: winsock get last error, if would block
    //    3.1 Unix: errno, if in progress
    //    3.2 select, if connection or timeout
    //    3.3 get socket option, if socket error
    // 4. sock_make_blocking

    // Configure expected system calls:
    // * make no blocking succeeds
    // + starting connection (no wait) returns with -1, errno = EINPROGRESS;
    // * connection succeeds
    // * make blocking succeeds

    // Mock used functions
    // -------------------
    // Unblock connection to return successful means don't wait on connect and
    // return immediately.
    umock::PupnpSockMock mock_pupnpSockObj;
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(1);
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    umock::Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    // select()
    EXPECT_CALL(mock_sys_selectObj,
                select(m_sockfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));

    // Connect to the given ip address. With unblocking this will
    // return with an error condition and errno = EINPROGRESS
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
#ifndef _WIN32
    // getsockopt() to get socket error
    EXPECT_CALL(mock_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(1);

#else
    umock::Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    // WSAGetLastError
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));
#endif

    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Test Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);

    // Test Unit with default blocked TCP connections
    // ----------------------------------------------
    unblock_tcp_connections = false;

    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .Times(1);
    // All other system calls are expected not to be called as set above.

    // Test Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
}

TEST_F(PrivateConnectFTestSuite, connect_immediately) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' succeeds.
    // * errno preset with EINVAL from any other old call.
    // * 'connect()' returns with 0, errno not modified.
    // * 'WSAGetLastError()' not called.
    // * 'select()' within Check_Connect_And_Wait_Connection() not called.
    // * 'getsockopt()' within Check_Connect_And_Wait_Connection() not called.
    // * 'sock_make_blocking()' succeeds to revert 'sock_make_no_blocking'.

    // Unblock connection to return successful means don't wait on connect and
    // return immediately.
    umock::PupnpSockMock mock_pupnpSockObj;
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    // Connect to the given ip address. We expect that it fails with
    // errno = ENETUNREACH (Network is unreachable).
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(Return(0));
    // getsockopt()
    EXPECT_CALL(mock_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);

#ifdef _WIN32
    umock::Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    // WSAGetLastError
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).Times(0);
#endif

    umock::Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    // select()
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);

    // Test Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);

    // Test Unit with default blocked TCP connections
    // ----------------------------------------------
    unblock_tcp_connections = false;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' not called.
    // * 'connect()' returns with 0, errno not modified.
    // * 'WSAGetLastError()' not called.
    // * 'select()' within Check_Connect_And_Wait_Connection() not called.
    // * 'getsockopt()' within Check_Connect_And_Wait_Connection() not called.
    // * 'sock_make_blocking()' not called.

    // sock_make_no_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(0);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(Return(0));
#ifdef _WIN32
    // WSAGetLastError
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).Times(0);
#endif
    // select()
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt()
    EXPECT_CALL(mock_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(0);

    // Test the Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
}

TEST_F(PrivateConnectFTestSuite, set_no_blocking_fails) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' returns with error, set errno = EINVAL.
    // * 'connect()' must not be called.
    // * select() within Check_Connect_And_Wait_Connection() must not be called.
    // * 'sock_make_blocking()' must not be called.

    // Unblock connection means don't wait on connect and return immediately.
    umock::PupnpSockMock mock_pupnpSockObj;
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    // sock_make_no_blocking(),
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, SOCKET_ERROR));
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(_)).Times(0);

    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    // connect()
    EXPECT_CALL(mock_socketObj, connect(_, _, _)).Times(0);

    umock::Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    // select()
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);

    // Test Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        SOCKET_ERROR);

    // Test Unit with default blocked TCP connections
    // ----------------------------------------------
    unblock_tcp_connections = false;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' must not be called.
    // * 'connect()' must be called successful.
    // * select() within Check_Connect_And_Wait_Connection() must not be called.
    // * 'sock_make_blocking()' must not be called.

    // sock_make_no_blocking(),
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(_)).Times(0);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .Times(1);

    // Test Unit
    EXPECT_EQ(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
}

TEST_F(PrivateConnectFTestSuite, connect_fails) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' succeeds.
    // * errno preset with EINVAL from any other old call.
    // * 'connect()' returns with SOCKET_ERROR, errno set to ENETUNREACH, resp.
    // * 'WSAGetLastError()' returns WSAENETUNREACH (Network is unreachable).
    // * 'select()' within Check_Connect_And_Wait_Connection() not called.
    // * 'getsockopt()' within Check_Connect_And_Wait_Connection() not called.
    // * 'sock_make_blocking()' succeeds to revert 'sock_make_no_blocking'.

    // Unblock connection to return successful means don't wait on connect and
    // return immediately.
    umock::PupnpSockMock mock_pupnpSockObj;
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    // Connect to the given ip address. We expect that it fails with
    // errno = ENETUNREACH (Network is unreachable).
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(ENETUNREACH, SOCKET_ERROR));
    // getsockopt()
    EXPECT_CALL(mock_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);

#ifdef _WIN32
    umock::Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    // WSAGetLastError
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAENETUNREACH));
#endif

    umock::Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    // select()
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": private_connect() must not return successful when the "
                     "connection fails.\n";
        EXPECT_EQ(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0); // Wrong!

    } else {

        EXPECT_NE(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0);
    }

    // Test Unit with default blocked TCP connections
    // ----------------------------------------------
    unblock_tcp_connections = false;

    // Configure expected system calls:
    // * 'sock_make_no_blocking()' not called.
    // * 'connect()' returns with SOCKET_ERROR, errno set to ENETUNREACH.
    // * 'WSAGetLastError()' not called.
    // * 'select()' within Check_Connect_And_Wait_Connection() not called.
    // * 'getsockopt()' within Check_Connect_And_Wait_Connection() not called.
    // * 'sock_make_blocking()' not called.

    // sock_make_no_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(0);
    // connect()
    EXPECT_CALL(mock_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(ENETUNREACH, SOCKET_ERROR));
#ifdef _WIN32
    // WSAGetLastError
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).Times(0);
#endif
    // select()
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt()
    EXPECT_CALL(mock_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);
    // sock_make_blocking()
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(0);

    // Test the Unit
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
}

TEST_F(PrivateConnectFTestSuite, Check_Connect_And_Wait_Connection_fails) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'make_no_blocking()' succeeds.
    // * errno preset with EINVAL from any other old call.
    // * 'connect()' returns "successful" with -1, errno EINPROGRESS
    // * check connection fails
    // * make blocking succeeds
}

#if false
TEST_F(PrivateConnectIp4FTestSuite, Check_Connect_And_Wait_Connection_fails) {
    // Configure expected system calls:
    // * 'make no blocking' succeeds
    // * errno preset with EINVAL from any other old call
    // * connect() (no blocking) returns successful with -1, errno EINPROGRESS
    // * check connection fails
    // * make blocking succeeds

    umock::PupnpSockMock mock_pupnpSockObj;
    // First unblock connection, means don't wait on connect and return
    // immediately. Returns successful, preset errno = EINVAL.
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));

    // Then connect to the given ip address. returns with -1 and errno =
    // EINPROGRESS.
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj,
                connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(m_saddrin)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must be set to the return value of
    // connect(). This will fail.
    PupnpHttpRwMock mock_pupnpHttpRwObj;
    umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
    EXPECT_CALL(mock_pupnpHttpRwObj,
                Check_Connect_And_Wait_Connection(m_socketfd, -1))
        .WillOnce(Return(-1));

    if (old_code) {
        // Set blocking mode. This should be executed to revert set no blocking
        // if possible, no matter if successful.
        ::std::cout << "  BUG! Blocking has been disabled so it should be "
                       "enabled as before.\n";
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd)).Times(0);

    } else if (github_actions) {
        ::std::cout << "[  SKIPPED ] Test on Github Actions\n";
        SUCCEED();
    } else {
        // Set blocking mode. This should be executed to revert set no blocking
        // if possible, no matter if successful.
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd)).Times(1);
    }

    // Test the Unit
    EXPECT_NE(
        private_connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(sockaddr_in)),
        0);
}

TEST_F(PrivateConnectIp4FTestSuite, sock_make_blocking_fails) {
    // Configure expected system calls:
    // * 'make no blocking' succeeds
    // * errno preset with EINVAL from any other old call
    // * connect() (no blocking) returns successful with -1, errno EINPROGRESS
    // * check connection succeeds
    // * make blocking fails

    umock::PupnpSockMock mock_pupnpSockObj;
    // First unblock connection, means don't wait on connect and return
    // immediately. Returns successful, preset errno = EINVAL.
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));

    // Then connect to the given ip address. Returns with -1 and errno =
    // EINPROGRESS.
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj,
                connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(m_saddrin)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must be set to the return value of
    // connect(). This will fail.
    PupnpHttpRwMock mock_pupnpHttpRwObj;
    umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
    EXPECT_CALL(mock_pupnpHttpRwObj,
                Check_Connect_And_Wait_Connection(m_socketfd, -1))
        .WillOnce(Return(-1));

    if (old_code) {
        // Set blocking mode. This should be executed to revert set no blocking
        // if possible, no matter if successful.
        ::std::cout << "  BUG! Blocking has been disabled so it should be "
                       "enabled as before.\n";
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd)).Times(0);

    } else if (github_actions) {
        ::std::cout << "[  SKIPPED ] Test on Github Actions\n";
        SUCCEED();
    } else {

        // Set blocking mode. This should be executed to revert set no blocking
        // if possible, no matter if successful.
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd)).Times(1);
    }

    // Test the Unit
    EXPECT_NE(
        private_connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(sockaddr_in)),
        0);
}

//
// ######################################
// Tests for Microsoft Windows
// ######################################
#ifdef _WIN32
// There are corresponding tests for Unix systems below. You may compare them to
// look for differences.

TEST(CheckConnectAndWaitConnectionIp4TestSuite, wrong_connect_retval) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // WSAGetLastError
    Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).Times(0);

    // Test the unit
    int connect_retval{0};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, connect_error) {
    // This file descriptor is assumed to be not valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // WSAGetLastError WSAEBADF = "File handle is not valid."
    Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).WillOnce(Return(WSAEBADF));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, select_times_out) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select() returns 0, that is timeout
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(0));
    // WSAGetLastError
    Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, select_error) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select() returns -1, that is failure
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // WSAGetLastError
    Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, sockoption_error) {
    if (old_code)
        ::std::cout << "  # There are no errors from socket options checked.\n";
    else
        GTEST_SKIP() << "  OPT: There are no errors from socket options "
                        "checked. This should be revised.";
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, getsockopt_error) {
    if (old_code)
        ::std::cout << "  # getsockopt() isn't used.\n";
    else
        GTEST_SKIP()
            << "  OPT: getsockopt() isn't used. This should be revised.";
}

//
// ######################################
// Tests for Unix
// ######################################
#else  // _WIN32
// There are corresponding tests for Microsoft Windows above. You may compare
// them to look for differences.

TEST(CheckConnectAndWaitConnectionIp4TestSuite, wrong_connect_retval) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(_, _, _, _, _)).Times(0);

    // Test the unit
    int connect_retval{0};
    errno = 0;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), 0);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, connect_error) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(_, _, _, _, _)).Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = ETIMEDOUT;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), 0);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, select_times_out) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(0));
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), -1);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, select_error) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select() returns -1, that is failure
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), -1);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, sockoption_error) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select() returns 1, that means one socket is ready for writing
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt(), the error is returned with SetArgPtrIntValue<3>(1)
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(1), Return(0)));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), -1);
}

TEST(CheckConnectAndWaitConnectionIp4TestSuite, getsockopt_error) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt() fails with returning -1
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .WillOnce(Return(-1));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    EXPECT_EQ(Check_Connect_And_Wait_Connection(socketfd, connect_retval), -1);
}
#endif // _WIN32
#endif // #if 0

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}

// vim: nowrap
