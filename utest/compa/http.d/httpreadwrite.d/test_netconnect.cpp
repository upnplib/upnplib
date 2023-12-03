// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-06

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include <pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp>
#else
#include <compa/src/genlib/net/http/httpreadwrite.cpp>
#endif

#include <upnplib/global.hpp>
#include <upnplib/sockaddr.hpp>

#include <utest/utest.hpp>
#include <umock/sys_socket_mock.hpp>
#include <umock/pupnp_sock_mock.hpp>
#include <umock/winsock2_mock.hpp>


namespace utest {

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

using ::upnplib::SSockaddr;


// ######################################
// testsuite for httpreadwrite
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
    const SOCKET m_sockfd{umock::sfd_base + 40};
    // Ip address structure
    SSockaddr m_saddr;

    // Needed mocked functions
    umock::PupnpSockMock m_pupnpSockObj;
    umock::Sys_socketMock m_sys_socketObj;
#ifdef _WIN32
    umock::Winsock2Mock m_winsock2Obj;
#endif

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
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(1);
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_sockfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));

    // Connect to the given ip address. With unblocking this will
    // return with an error condition and errno = EINPROGRESS
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    // getsockopt() to get socket error
#ifdef _WIN32
    if (old_code)
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, _, _))
            .Times(0); // Wrong!
    else
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, _, _))
            .Times(1);
#else
    EXPECT_CALL(m_sys_socketObj,
                getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, _, _))
        .Times(1);
#endif

#ifdef _WIN32
    umock::Winsock2 winsock2_injectObj(&m_winsock2Obj);
    // WSAGetLastError()
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
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
    EXPECT_CALL(m_sys_socketObj,
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
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    // Connect to the given ip address. We expect that it fails with
    // errno = ENETUNREACH (Network is unreachable).
    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(Return(0));
    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);

#ifdef _WIN32
    umock::Winsock2 winsock2_injectObj(&m_winsock2Obj);
    // WSAGetLastError
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError()).Times(0);
#endif

    // select()
    EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);

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
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(0);
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(Return(0));
#ifdef _WIN32
    // WSAGetLastError
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError()).Times(0);
#endif
    // select()
    EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);
    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(0);

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
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    // sock_make_no_blocking(),
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, SOCKET_ERROR));
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(_)).Times(0);

    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
    // connect()
    EXPECT_CALL(m_sys_socketObj, connect(_, _, _)).Times(0);
    // select()
    EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);

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
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(_)).Times(0);
    // connect()
    EXPECT_CALL(m_sys_socketObj,
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
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    // sock_make_no_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .WillOnce(SetErrnoAndReturn(EINVAL, 0));
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(1);

    // Connect to the given ip address. We expect that it fails with
    // errno = ENETUNREACH (Network is unreachable).
    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(ENETUNREACH, SOCKET_ERROR));
    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);

#ifdef _WIN32
    umock::Winsock2 winsock2_injectObj(&m_winsock2Obj);
    // WSAGetLastError
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAENETUNREACH));
#endif

    // select()
    EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);

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
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd)).Times(0);
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .WillOnce(SetErrnoAndReturn(ENETUNREACH, SOCKET_ERROR));
#ifdef _WIN32
    // WSAGetLastError
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError()).Times(0);
#endif
    // select()
    EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);
    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(0);

    // Test the Unit
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
}

TEST_F(PrivateConnectFTestSuite, select_fails) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'make_no_blocking()' succeeds.
    // * errno preset with EINVAL from any other old call.
    // * 'connect()' returns "successful" with -1, errno EINPROGRESS
    // * _WIN32: 'WSAGetLastError()' returns WSAEWOULDBLOCK.
    // * 'select()' within 'Check_Connect_And_Wait_Connection()' fails.
    // * 'getsockopt()' within 'Check_Connect_And_Wait_Connection()' not called.
    // * make blocking succeeds

    // Inject needed mock objects into the productive code.
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
#ifdef _WIN32
    umock::Winsock2 winsock2_injectObj(&m_winsock2Obj);
#endif

    // sock_make_no_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .Times(2)
        .WillRepeatedly(SetErrnoAndReturn(EINVAL, 0));
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .Times(2)
        .WillRepeatedly(SetErrnoAndReturn(EINPROGRESS, -1));
#ifdef _WIN32
    // WSAGetLastError()
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
        .Times(2)
        .WillRepeatedly(Return(WSAEWOULDBLOCK));
#endif
    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, _, _, _, _)).Times(0);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": private_connect() must always sock_make_blocking() "
                     "again, also when the connection fails.\n";
        // sock_make_blocking()
        EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd))
            .Times(0); // Wrong!

    } else {

        // sock_make_blocking()
        EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(2);
    }

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_sockfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(SOCKET_ERROR));

    // Test the Unit
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_sockfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(0)); // 0 indicates "timeout" and is an error.

    // Test the Unit
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);

    // Test Unit with default blocking TCP connections
    // -----------------------------------------------
    // There is no equivalent call. Only 'connect()' is called but never
    // 'select()'. Successful and failed blocking 'connect()' is already tested.
}

TEST_F(PrivateConnectFTestSuite, getsockopt_fails) {
    // Test Unit with unblocked TCP connections
    // ----------------------------------------
    unblock_tcp_connections = true;

    // Configure expected system calls:
    // * 'make_no_blocking()' succeeds.
    // * errno preset with EINVAL from any other old call.
    // * 'connect()' returns "successful" with -1, errno EINPROGRESS
    // * _WIN32: 'WSAGetLastError()' returns WSAEWOULDBLOCK.
    // * 'select()' within 'Check_Connect_And_Wait_Connection()' succeeds.
    // * 'getsockopt()' within 'Check_Connect_And_Wait_Connection()' fails.
    // * make blocking succeeds

    // Inject needed mock objects into the production code.
    umock::PupnpSock pupnp_sock_injectObj(&m_pupnpSockObj);
    umock::Sys_socket sys_socket_injectObj(&m_sys_socketObj);
#ifdef _WIN32
    umock::Winsock2 winsock2_injectObj(&m_winsock2Obj);
#endif

    // Common expectations on all platforms
    // ------------------------------------
    // sock_make_no_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_no_blocking(m_sockfd))
        .Times(2)
        .WillRepeatedly(SetErrnoAndReturn(EINVAL, 0));
    // connect()
    EXPECT_CALL(m_sys_socketObj,
                connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)))
        .Times(2)
        .WillRepeatedly(SetErrnoAndReturn(EINPROGRESS, -1));
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_sockfd + 1, NULL, NotNull(), NULL, NotNull()))
        .Times(2)
        .WillRepeatedly(Return(1));

    // Always show BUGFIX Messages for all platforms.
    if (old_code) {
        // sock_make_blocking()
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": private_connect() on Unix platforms must always "
                     "sock_make_blocking(), also when connection fails.\n";
        // Test Unit on WIN32
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": private_connect() on WIN32 must not return successful "
                     "when the connection fails.\n";
        // getsockopt()
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": getsockopt() is never called on WIN32 but it should.\n";
    }

    // Expectations on Microsoft Windows
    // ---------------------------------
#ifdef _WIN32
    // WSAGetLastError()
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
        .Times(2)
        .WillRepeatedly(Return(WSAEWOULDBLOCK));
    // sock_make_blocking()
    EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(2);

    if (old_code) {
        // getsockopt()
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR,
                                                NotNull(), NotNull()))
            .Times(0); // Wrong! Message see above.

        // Test Unit
        EXPECT_EQ(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0); // Wrong! Message see above.
        EXPECT_EQ(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0); // Wrong! Message see above.

    } else {

        // getsockopt()
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR,
                                                NotNull(), NotNull()))
            .WillOnce(Return(-1))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(1), Return(0)));

        // Test Unit
        EXPECT_NE(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0);
        EXPECT_NE(private_connect(m_sockfd, (sockaddr*)&m_saddr.ss,
                                  sizeof(m_saddr.ss)),
                  0);
    }

#else // _WIN32

    // Expectations on platforms that are not Microsoft Windows
    // --------------------------------------------------------
    // sock_make_blocking()
    if (old_code)
        EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd))
            .Times(0); // Wrong!
    else
        EXPECT_CALL(m_pupnpSockObj, sock_make_blocking(m_sockfd)).Times(2);

    // getsockopt()
    EXPECT_CALL(m_sys_socketObj, getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR,
                                            NotNull(), NotNull()))
        .WillOnce(Return(-1))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(1), Return(0)));

    // Test Unit
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
    EXPECT_NE(
        private_connect(m_sockfd, (sockaddr*)&m_saddr.ss, sizeof(m_saddr.ss)),
        0);
#endif

    // Test Unit with default blocking TCP connections
    // -----------------------------------------------
    // There is no equivalent call. Only 'connect()' is called but never
    // 'select()'. Successful and failed blocking 'connect()' is already tested.
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}

// vim: nowrap
