// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp"
#include "upnplib/src/net/http/httpreadwrite.cpp"

#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"
#include "umock/sys_socket_mock.hpp"
#include "umock/pupnp_sock_mock.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

using ::upnplib::testing::SetArgPtrIntValue;

namespace compa {

bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// ######################################
// Mocked system calls
// ######################################
class Sys_selectMock : public umock::Sys_selectInterface {
  public:
    virtual ~Sys_selectMock() override {}
    MOCK_METHOD(int, select,
                (SOCKET nfds, fd_set* readfds, fd_set* writefds,
                 fd_set* exceptfds, struct timeval* timeout),
                (override));
};

class PupnpHttpRwMock : public umock::PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwMock() override = default;
    MOCK_METHOD(int, private_connect,
                (SOCKET sockfd, const struct sockaddr* serv_addr,
                 socklen_t addrlen),
                (override));
    MOCK_METHOD(int, Check_Connect_And_Wait_Connection,
                (SOCKET sock, int connect_res), (override));
};

//
// ######################################
// testsuite for Ip4 httpreadwrite
// ######################################
#if false
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

//
class PrivateConnectIp4FTestSuite : public ::testing::Test {
  protected:
    // Fictive socket file descriptor for mocking.
    const int m_socketfd{513};
    // Ip address structure
    ::sockaddr_in m_saddrin{};

    PrivateConnectIp4FTestSuite() {
        m_saddrin.sin_family = AF_INET;
        m_saddrin.sin_port = htons(80);
        m_saddrin.sin_addr.s_addr = ::inet_addr("192.168.192.168");
    }
};

TEST_F(PrivateConnectIp4FTestSuite, successful_connect) {
    // Steps as given by the Unit:
    // 1. sock_make_no_blocking
    // 2. try to connect
    // 3. check connection an and wait until connected or timed out
    // 4. sock_make_blocking

    // Configure expected system calls:
    // * make no blocking succeeds
    // + starting connection (no wait) returns with -1, errno = EINPROGRESS;
    // * connection succeeds
    // * make blocking succeeds

    umock::PupnpSockMock mock_pupnpSockObj;
    // First unblock connection, means don't wait on connect and return
    // immediately.
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
        .WillOnce(Return(0));

    // Then connect to the given ip address. With unblocking this will return
    // with an error condition and errno = EINPROGRESS
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj,
                connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(m_saddrin)))
        .WillOnce(Return(-1));

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must return value of connect().
    PupnpHttpRwMock mock_pupnpHttpRwObj;
    umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
    EXPECT_CALL(mock_pupnpHttpRwObj,
                Check_Connect_And_Wait_Connection(m_socketfd, -1))
        .WillOnce(Return(0));
    // Set blocking mode
    EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd))
        .WillOnce(Return(0));

    // Test the Unit
    errno = EINPROGRESS;
    // errno = ENETUNREACH; // Network is unreachable.
    EXPECT_EQ(
        private_connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(sockaddr_in)),
        0);
}

TEST_F(PrivateConnectIp4FTestSuite, set_no_blocking_fails) {
    // Configure expected system calls:
    // * 'make no blocking' fails
    // * errno preset with EINVAL from any other old call
    // * connect() (blocking) returns with 0, errno untouched
    // * connection succeeds
    // * make blocking succeeds

    umock::Sys_socketMock mock_socketObj;

    if (old_code) {
        umock::PupnpSockMock mock_pupnpSockObj;
        // First unblock connection, means don't wait on connect and return
        // immediately. Returns with error, preset errno = EINVAL.
        umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
        EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
            .WillOnce(SetErrnoAndReturn(
                EINVAL, 10098)); // On MS Windows are big error numbers

        // Then connect to the given ip address. With unblocked this will return
        // with an error condition and errno = EINPROGRESS. But in this case
        // with failed unblock we expect a blocked but successful connection.
        umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
        ::std::cout << "  BUG! Disable blocking has failed so it should not "
                       "try to connect.\n";
        EXPECT_CALL(mock_socketObj, connect(m_socketfd, (sockaddr*)&m_saddrin,
                                            sizeof(m_saddrin)))
            .WillOnce(Return(0));

        // Check the connection with a short timeout (default 5s) and return if
        // ready to send. Arg1 (0 based) must be set to the return value of
        // connect().
        ::std::cout << "  BUG! Disable blocking has failed so it should not "
                       "wait for a connection.\n";
        PupnpHttpRwMock mock_pupnpHttpRwObj;
        umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
        EXPECT_CALL(mock_pupnpHttpRwObj,
                    Check_Connect_And_Wait_Connection(m_socketfd, 0))
            .WillOnce(Return(0));

        // Set blocking mode
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd))
            .WillOnce(Return(0));

        // Test the Unit
        // errno = ENETUNREACH; // Network is unreachable.
        EXPECT_EQ(private_connect(m_socketfd, (sockaddr*)&m_saddrin,
                                  sizeof(sockaddr_in)),
                  0);

    } else if (github_actions) {
        ::std::cout << "[  SKIPPED ] Test on Github Actions\n";
        SUCCEED();
    } else {

        umock::PupnpSockMock mock_pupnpSockObj;
        // Unblock connection. Returns with error, preset errno = EINVAL.
        umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
        EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
            .WillOnce(SetErrnoAndReturn(
                EINVAL, 10098)); // On MS Windows are big error numbers

        // Don't try to connect to the given ip address.
        umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
        EXPECT_CALL(mock_socketObj, connect(m_socketfd, (sockaddr*)&m_saddrin,
                                            sizeof(m_saddrin)))
            .Times(0);

        // Don't try to wait for that connection.
        PupnpHttpRwMock mock_pupnpHttpRwObj;
        umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
        EXPECT_CALL(mock_pupnpHttpRwObj,
                    Check_Connect_And_Wait_Connection(m_socketfd, 0))
            .Times(0);

        // Set blocking mode. That's the old workflow. With re-engeneering we
        // will always have unblocking mode set.
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd))
            .WillOnce(Return(0));

        // Test the Unit
        EXPECT_NE(private_connect(m_socketfd, (sockaddr*)&m_saddrin,
                                  sizeof(sockaddr_in)),
                  0);
    }
}

TEST_F(PrivateConnectIp4FTestSuite, connect_fails) {
    // Configure expected system calls:
    // * 'make no blocking' succeeds
    // * errno preset with EINVAL from any other old call
    // * connect() (no blocking) returns with -1, errno set to ENETUNREACH
    //   (Network is unreachable).
    // * check connection must also fail if connect() fails
    // * make blocking succeeds

    umock::PupnpSockMock mock_pupnpSockObj;
    // First unblock connection, means don't wait on connect and return
    // immediately. Returns successful, preset errno = EINVAL.
    umock::PupnpSock pupnp_sock_injectObj(&mock_pupnpSockObj);
    EXPECT_CALL(mock_pupnpSockObj, sock_make_no_blocking(m_socketfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));

    // Then connect to the given ip address. We expect that it fails with
    // errno = ENETUNREACH (Network is unreachable).
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj,
                connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(m_saddrin)))
        .WillOnce(SetErrnoAndReturn(ENETUNREACH, -1));

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must be set to the return value of
    // connect(). Because connect() failed, this must also fail.
    PupnpHttpRwMock mock_pupnpHttpRwObj;
    umock::PupnpHttpRw pupnp_httprw_injectObj(&mock_pupnpHttpRwObj);
    EXPECT_CALL(mock_pupnpHttpRwObj,
                Check_Connect_And_Wait_Connection(m_socketfd, -1))
        .WillOnce(Return(-1));

    if (old_code) {
        // Set blocking mode
        ::std::cout << "  BUG! Blocking has been disabled so it should be "
                       "enabled as before.\n";
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd)).Times(0);

    } else if (github_actions) {
        ::std::cout << "[  SKIPPED ] Test on Github Actions\n";
        SUCCEED();
    } else {

        // Set blocking mode
        EXPECT_CALL(mock_pupnpSockObj, sock_make_blocking(m_socketfd))
            .WillOnce(Return(0));
    }

    // Test the Unit
    EXPECT_NE(
        private_connect(m_socketfd, (sockaddr*)&m_saddrin, sizeof(sockaddr_in)),
        0);
}

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

// Mocked system calls on MS Windows
// ---------------------------------
class Winsock2Mock : public umock::Winsock2Interface {
  public:
    virtual ~Winsock2Mock() override = default;
    MOCK_METHOD(int, WSAGetLastError, (), (override));
};

TEST(CheckConnectAndWaitConnectionIp4TestSuite, successful_connect) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // WSAGetLastError
    Winsock2Mock mock_winsock2Obj;
    umock::Winsock2 winsock2_injectObj(&mock_winsock2Obj);
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

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

TEST(CheckConnectAndWaitConnectionIp4TestSuite, successful_connect) {
    // This file descriptor is assumed to be valid.
    SOCKET socketfd{258};

    // Configure expected system calls.
    // select()
    Sys_selectMock mock_sys_selectObj;
    umock::Sys_select sys_select_injectObj(&mock_sys_selectObj);
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(0), Return(0)));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

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
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
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
    // getsockopt
    umock::Sys_socketMock mock_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mock_socketObj);
    EXPECT_CALL(mock_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                           NotNull(), NotNull()))
        .Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
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
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
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
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}
#endif // _WIN32

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}

// vim: nowrap
