// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-22

// Helpful link for ip address structures:
// https://stackoverflow.com/q/76548580/5014688

#include <upnp.hpp>

#include <upnplib/global.hpp>
#include <upnplib/upnptools.hpp> // For errStr??
#include <upnplib/sockaddr.hpp>
#include <upnplib/socket.hpp>

#include <pupnp/upnpdebug.hpp>

#include <utest/utest.hpp>

#include <umock/unistd_mock.hpp>
#include <umock/sys_socket_mock.hpp>
#include <umock/ssl_mock.hpp>

#include <fcntl.h>

#define sockaddr_storage upnplib::sockaddr_t
#include <sock.hpp>

namespace utest {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::ExitedWithCode;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::IsNull;
using ::testing::Ne;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::no_adl::Conditional;

using ::upnplib::errStr;
using ::upnplib::errStrEx;
using ::upnplib::g_dbug;
using ::upnplib::SSockaddr;

using ::pupnp::CLogging;

using ::umock::sfd_base;


// testsuite for the sock module
//==============================
class SockFTestSuite : public ::testing::Test {
  protected:
    // clang-format off
    // Instantiate mocking objects.
    StrictMock<umock::Sys_socketMock> m_sys_socketObj;
    // Inject the mocking objects into the tested code.
    umock::Sys_socket sys_socket_injectObj = umock::Sys_socket(&m_sys_socketObj);
    // clang-format on

    StrictMock<umock::UnistdMock> m_unistdObj;
    umock::Unistd unistd_injectObj = umock::Unistd(&m_unistdObj);
#ifdef UPNP_ENABLE_OPEN_SSL
    StrictMock<umock::SslMock> m_sslObj;
    umock::Ssl ssl_injectObj = umock::Ssl(&m_sslObj);
#endif
};
typedef SockFTestSuite SockFDeathTest;


TEST(SockTestSuite, sock_init_successful) {
    constexpr SOCKET sockfd{sfd_base + 62};
    ::SOCKINFO sockinfo;

    // Test Unit
    int ret_sock_init = sock_init(&sockinfo, sockfd);
    EXPECT_EQ(ret_sock_init, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_init, UPNP_E_SUCCESS);

    EXPECT_EQ(sockinfo.socket, sockfd);
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_family, 0);
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_port, 0);
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_addr.s_addr,
              static_cast<in_addr_t>(0));
}

TEST(SockDeathTest, sock_init_with_no_info) {
    constexpr SOCKET sockfd{sfd_base + 63};

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Function should not segfault or abort with failed "
                     "assert().\n";
        EXPECT_DEATH(sock_init(nullptr, sockfd), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((sock_init(nullptr, sockfd), exit(0)), ExitedWithCode(0),
                    ".*");
        int ret_sock_init = sock_init(nullptr, sockfd);
        EXPECT_EQ(ret_sock_init, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_sock_init, UPNP_E_INVALID_PARAM);
    }
}

TEST(SockTestSuite, sock_init_with_ip_successful) {
    constexpr SOCKET sockfd{sfd_base + 64};
    ::SOCKINFO sockinfo;

    // Provide a sockaddr_in structure
    SSockaddr saddrObj;
    saddrObj = "192.168.192.168:80";

    // Test Unit
    int ret_sock_init_with_ip =
        sock_init_with_ip(&sockinfo, sockfd, &saddrObj.sa);
    EXPECT_EQ(ret_sock_init_with_ip, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_init_with_ip, UPNP_E_SUCCESS);

    EXPECT_EQ(sockinfo.socket, sockfd);
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_family, AF_INET);
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_port, htons(80));
    EXPECT_EQ(sockinfo.foreign_sockaddr.sin.sin_addr.s_addr,
              saddrObj.sin.sin_addr.s_addr);
}

TEST(SockDeathTest, sock_init_with_ip_but_no_ip) {
    // Error condition with no info structure is tested with
    // sock_init_with_no_info.
    constexpr SOCKET sockfd{sfd_base + 65};
    ::SOCKINFO sockinfo;

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Function should not segfault.\n";
        EXPECT_DEATH(sock_init_with_ip(&sockinfo, sockfd, nullptr), ".*");
    } else {

        // This expects NO segfault.
        ASSERT_EXIT((sock_init_with_ip(&sockinfo, sockfd, nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_sock_init_with_ip =
            sock_init_with_ip(&sockinfo, sockfd, nullptr);
        EXPECT_EQ(ret_sock_init_with_ip, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_sock_init_with_ip, UPNP_E_INVALID_PARAM);
    }
}

TEST_F(SockFTestSuite, sock_destroy_successful) {
    constexpr SOCKET sockfd{sfd_base + 66};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // shutdown is successful
    EXPECT_CALL(m_sys_socketObj, shutdown(sockfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close is successful
    EXPECT_CALL(m_unistdObj, CLOSE_SOCKET_P(sockfd)).WillOnce(Return(0));

    // Test Unit
    int ret_sock_destroy = sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH);
    EXPECT_EQ(ret_sock_destroy, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_destroy, UPNP_E_SUCCESS);
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_ok_close_fails) {
    constexpr SOCKET sockfd{sfd_base + 67};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // shutdown is successful
    EXPECT_CALL(m_sys_socketObj, shutdown(sockfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close fails on _WIN32 with positive error number
    EXPECT_CALL(m_unistdObj, CLOSE_SOCKET_P(sockfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Test Unit
    int ret_sock_destroy;
    if (old_code) {
        // BUG! closesocket on _WIN32 does not return -1 on error, but positive
        // numbers. Check of -1 results on win32 in UPNP_E_SUCCESS instead of
        // UPNP_E_SOCKET_ERROR. Check must be on != 0. --Ingo
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": Successful socket shutdown but close != 0 should fail.\n";
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS) // Wrong!
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);

    } else {

        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_fails_close_ok) {
    constexpr SOCKET sockfd{sfd_base + 68};

    // shutdown should fail
    EXPECT_CALL(m_sys_socketObj, shutdown(sockfd, /*SHUT_RDWR*/ SD_BOTH))
        // First call: shutdown a not connected connection is not an error.
        .WillOnce(SetErrPtblAndReturn(ENOTCONNP, SOCKET_ERROR))
        // Second call: shutdown error on connected connection.
        .WillOnce(SetErrPtblAndReturn(EBADFP, SOCKET_ERROR));
    // close is successful
    EXPECT_CALL(m_unistdObj, CLOSE_SOCKET_P(sockfd))
        .Times(2)
        .WillRepeatedly(Return(0));

    // Test Unit
    ::SOCKINFO sockinfo;
    int ret_sock_destroy;

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": Failing socket shutdown with successful close should fail.\n";
        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(ret_sock_destroy, UPNP_E_SUCCESS);

        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS) // Wrong!
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);

    } else {

        // First call: shutdown a not connected connection is not an error.
        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(ret_sock_destroy, UPNP_E_SUCCESS);

        // Second call: shutdown error on connected connection.
        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_and_close_fails) {
    constexpr SOCKET sockfd{sfd_base + 69};

    // shutdown should fail
    EXPECT_CALL(m_sys_socketObj, shutdown(sockfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(SetErrPtblAndReturn(EBADFP, SOCKET_ERROR));
    // close fails on _WIN32 with positive error number
    EXPECT_CALL(m_unistdObj, CLOSE_SOCKET_P(sockfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Test Unit
    ::SOCKINFO sockinfo;
    int ret_sock_destroy;

    if (old_code) {
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": Failing socket shutdown and close should fail.\n";
        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);

    } else {

        sock_init(&sockinfo, sockfd);
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_destroy, UPNP_E_SOCKET_ERROR);
    }
}

#ifdef UPNP_ENABLE_OPEN_SSL
TEST_F(SockFTestSuite, sock_read_ssl_within_timeout_successful) {
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};

    // Configure expected system calls that will return a received message.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                       Field(&::timeval::tv_sec, Eq(timeout))))
        .Times(2)
        // Consider timeout to be undefined after select() returns.
        .WillRepeatedly(DoAll(StructSetToArg<4>(0xAA), Return(1)));

    constexpr char received_msg[]{
        "Mocked received TCP message within timeout."};
    // SSL_read()
    EXPECT_CALL(m_sslObj, SSL_read(_, _, _))
        .WillOnce(DoAll(StrCpyToArg<1>(received_msg),
                        Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));
    // recv()
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(DoAll(StrCpyToArg<1>(received_msg),
                        Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));
#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) {
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .Times(2)
            .WillRepeatedly(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _))
            .Times(2);
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _))
            .Times(2);
    }
#endif

    int timeoutSecs{timeout}; // Will be modified to used time
    char buffer[sizeof(received_msg)]{};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit unprotected
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, static_cast<int>(sizeof(received_msg)))
        << errStr(ret_sock_read);
    EXPECT_STREQ(buffer, received_msg);
    // Don't know for what next is good. I haven't found any useful
    // documentation about this "used time" and it is not used anywhere in the
    // production code. Here it is only given for compatibility.
    EXPECT_GE(timeoutSecs, timeout - 1);

    // Test Unit with ssl
    sockinfo.ssl = reinterpret_cast<SSL*>(1);
    ret_sock_read = sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_STREQ(buffer, received_msg);
    EXPECT_GE(timeoutSecs, timeout - 1);
}

#else

TEST_F(SockFTestSuite, sock_read_within_timeout_successful) {
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};

    // Configure expected system calls that will return a received message.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                       Field(&::timeval::tv_sec, Eq(timeout))))
        // Consider timeout to be undefined after select() returns.
        .WillOnce(DoAll(StructSetToArg<4>(0xAA), Return(1)));
    // recv()
    constexpr char received_msg[]{
        "Mocked received TCP message within timeout."};
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(DoAll(StrCpyToArg<1>(received_msg),
                        Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));
#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) { // Scope InSequence
        InSequence seq;
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _));
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _));
    } // End scope InSequence
#endif

    int timeoutSecs{timeout}; // Will be modified to used time
    char buffer[sizeof(received_msg)]{};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, static_cast<int>(sizeof(received_msg)))
        << errStr(ret_sock_read);
    EXPECT_STREQ(buffer, received_msg);
    // Don't know for what next is good. I haven't found any useful
    // documentation about this "used time" and it is not used anywhere in the
    // production code. Here it is only given for compatibility.
    EXPECT_GE(timeoutSecs, timeout - 1);
}
#endif

TEST_F(SockFTestSuite, sock_read_no_timeout_successful) {
    constexpr SOCKET sockfd{3};
    // timeout -1 blocks indefinitely waiting for a socket descriptor to become
    // ready.
    constexpr time_t timeout{-1};

    // Configure expected system calls that will return a received message.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), NotNull(), IsNull(), IsNull()))
        .WillOnce(Return(1));
    // recv()
    constexpr char received_msg[]{"Mocked received TCP message no timeout."};
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(DoAll(StrCpyToArg<1>(received_msg),
                        Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));
#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) { // Scope InSequence
        InSequence seq;
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _));
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _));
    } // End scope InSequence
#endif

    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{timeout}; // Will be modified to used time
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, static_cast<int>(sizeof(received_msg)))
        << errStr(ret_sock_read);
    EXPECT_STREQ(buffer, received_msg);
    if (g_dbug)
        std::cout << "DEBUG: time used = " << timeoutSecs << ".\n";
}

TEST_F(SockFTestSuite, sock_read_with_connection_error) {
    // Provide needed environment.
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);
    char buffer[1]{'\xAA'};

    // Configure expected system calls.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                       Field(&::timeval::tv_sec, Eq(timeout))))
        // Consider timeout to be undefined after select() returns.
        .WillOnce(DoAll(StructSetToArg<4>(0xAA),
                        SetErrPtblAndReturn(EBADFP, SOCKET_ERROR)));

    int timeoutSecs{timeout};

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_read, UPNP_E_SOCKET_ERROR);
    EXPECT_EQ(*buffer, '\xAA');
}

TEST_F(SockFTestSuite, sock_read_signal_catched) {
    // A signal like ^C should not interrupt reading. When catching it, reading
    // is restarted. So we expect in this test that 'select()' is called
    // two times.
    // Managing SIGPIPE is not required on reading network data so you will not
    // find this check on new code. SIGPIPE may only occur on writing network
    // data.
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};
    int timeoutSecs{timeout}; // Will be modified to used time
    constexpr char received_msg[]{
        "Mocked received TCP message after signal catched."};
    char buffer[sizeof(received_msg)]{};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Configure expected system calls.
#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) { // Scope InSequence
        // Managing SIGPIPE (not required).
        InSequence seq;
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _));
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _));
    }
#endif

    if (old_code)
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": Unit must not fail reading network data on win32 due "
                       "to wrong error checking.\n";
#ifdef _MSC_VER
    if (old_code) {
        // select
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Eq(timeout))))
            // Consider timeout to be undefined after select() returns.
            .WillOnce(DoAll(StructSetToArg<4>(0xAA),
                            SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR)));

        // Test Unit
        int ret_sock_read =
            sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR) // Wrong!
            << errStr(ret_sock_read);
        EXPECT_STREQ(buffer, ""); // Wrong!

    } else {

        // select
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Eq(timeout))))
            // Consider timeout to be undefined after select() returns.
            .WillOnce(DoAll(StructSetToArg<4>(0xAA), // Signal catched
                            SetErrPtblAndReturn(EINTRP, SOCKET_ERROR)))
            .WillOnce(DoAll(StructSetToArg<4>(0xAA), // Message received
                            SetErrPtblAndReturn(EINTRP, 1)));
        // recv()
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
            .WillOnce(
                DoAll(StrCpyToArg<1>(received_msg),
                      Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));

        // Test Unit
        int ret_sock_read =
            sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

        EXPECT_EQ(ret_sock_read, sizeof(received_msg)) << errStr(ret_sock_read);
        EXPECT_STREQ(buffer, received_msg);
    }
#endif

    if (old_code)
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": Unit must not read network data on unix with an "
                       "undefined timeout.\n";
#ifndef _MSC_VER
    if (old_code) {
        // select
        // Blocking select() is interrupted by a signal that is not for us.
        // This is ignored and reading will continue with a new select(). The
        // second attempt with the same timeout should succeed. Here the second
        // attempt uses the undefined timeout modified by the first attempt.
        // That is wrong.
        // First attempt that is interrupted with EINTR.
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Eq(timeout))))
            // Consider timeout to be undefined after select() returns. Fill
            // with 0x55 will give a big positive number.
            .WillOnce(DoAll(StructSetToArg<4>(0x55), // Signal catched
                            SetErrPtblAndReturn(EINTRP, SOCKET_ERROR)));
        // Second attempt with undefined timeout. For this test case I assume a
        // positve timeout (filled with 0x55) that will block until we get a
        // socket ready to read.
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Ne(timeout)))) // Wrong!
            .WillOnce(
                DoAll(StructSetToArg<4>(0xAA), SetErrPtblAndReturn(EINTRP, 1)));

    } else {

        // select
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Eq(timeout))))
            // Consider timeout to be undefined after select() returns.
            .WillOnce(DoAll(StructSetToArg<4>(0xAA), // Signal catched
                            SetErrPtblAndReturn(EINTRP, SOCKET_ERROR)))
            .WillOnce(DoAll(StructSetToArg<4>(0xAA), // Message received
                            SetErrPtblAndReturn(EINTRP, 1)));
    }

    // recv()
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(DoAll(StrCpyToArg<1>(received_msg),
                        Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, sizeof(received_msg)) << errStr(ret_sock_read);
    EXPECT_STREQ(buffer, received_msg);
#endif
}

TEST_F(SockFTestSuite, sock_read_with_receiving_error) {
    // Configure expected system calls that will return a received message.
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                       Field(&::timeval::tv_sec, Eq(timeout))))
        // Consider timeout to be undefined after select() returns.
        .WillOnce(DoAll(StructSetToArg<4>(0xAA), Return(1)));
    // recv()
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) { // Scope InSequence
        InSequence seq;
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _));
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _));
    } // End scope InSequence
#endif

    char buffer[2]{"\xAA"};
    int timeoutSecs{timeout};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

    EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_read, UPNP_E_SOCKET_ERROR);
    EXPECT_STREQ(buffer, "\xAA");
}

TEST(SockDeathTest, sock_read_with_invalid_pointer_to_socket_info) {
    char buffer[1]{};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    if (old_code) {
        ::std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": A nullptr to a socket info structure must not segfault.\n";
        EXPECT_DEATH(sock_read(nullptr, buffer, sizeof(buffer), &timeoutSecs),
                     ".*");

    } else {

        ASSERT_EXIT(
            (sock_read(nullptr, buffer, sizeof(buffer), &timeoutSecs), exit(0)),
            ExitedWithCode(0), ".*")
            << "  # A nullptr to a socket info structure must not segfault.";
        int ret_sock_read =
            sock_read(nullptr, buffer, sizeof(buffer), &timeoutSecs);
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_read, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_read_with_nullptr_to_buffer_or_0_byte_length) {
    // This should be a valid condition and indicating that there is nothing to
    // receive. With no pointer to a buffer, the buffer length doesn't matter.
    // With buffer pointer but 0 byte length, the buffer doesn't mätter. But
    // to stay compatible with old code I will also expect both to be an error.
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};
    int timeoutSecs{timeout};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    if (old_code) {
        // select()
        if (g_dbug)
            ::std::cout
                << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                           Field(&::timeval::tv_sec, Eq(timeout))))
            // Consider timeout to be undefined after select() returns.
            .Times(2)
            .WillRepeatedly(DoAll(StructSetToArg<4>(0xAA), Return(1)));

        // recv()
        if (g_dbug)
            ::std::cout
                << "  OPT: It is not needed to call system function 'recv()' "
                   "in this case.\n";
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, _, _, _))
            .WillOnce(SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR));
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), 0, _))
            .WillOnce(SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .Times(2)
            .WillRepeatedly(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _))
            .Times(2);
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _))
            .Times(2);
#endif
        // Test Unit
        int ret_sock_read = sock_read(&sockinfo, nullptr, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR);
        // ::std::cout
        //     << CYEL "[ FIX      ] " CRES << __LINE__
        //     << ": nullptr to buffer should receive number of bytes = 0 but
        //     not "
        //     << errStr(ret_sock_read) << ".\n";

        char buffer[]{""};
        ret_sock_read = sock_read(&sockinfo, buffer, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR); // Wrong!
        EXPECT_STREQ(buffer, "");
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": 0 byte buffer length should receive number of bytes "
                       "= 0 but not "
                    << errStr(ret_sock_read) << ".\n";

    } else {

        // Test Unit with nullptr to buffer
        // To be compatible with old code I accept UPNP_E_SOCKET_ERROR instead
        // of 0 bytes received. --Ingo
        int ret_sock_read = sock_read(&sockinfo, nullptr, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_read, UPNP_E_SOCKET_ERROR);
        // EXPECT_EQ(ret_sock_read, 0)
        //     << "  # Should receive number of bytes = 0"
        //     << " but not " << errStr(ret_sock_read) << ".";

        // Test Unit with with 0 byte buffer size.
        char buffer[]{""};
        ret_sock_read = sock_read(&sockinfo, buffer, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, 0)
            << "  # Should receive number of bytes = 0 but not "
            << errStr(ret_sock_read) << ".";
        EXPECT_STREQ(buffer, "");
    }
}

TEST_F(SockFDeathTest, sock_read_with_nullptr_to_timeout_value) {
    // Specifying a nullptr for the timeout value results in using the UPnP+
    // default response timeout.
    constexpr SOCKET sockfd{3};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);
    constexpr char received_msg[]{
        "Mocked received TCP message with nullptr to timeout value."};
    char buffer[sizeof(received_msg)]{};

    if (old_code) {
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": A nullptr to the timeout value must not segfault.\n";
        // Test Unit
        ASSERT_DEATH(sock_read(&sockinfo, buffer, sizeof(buffer), nullptr), "");

    } else {

        // Configure expected system calls that will return a received message.
        // select()
        EXPECT_CALL(
            m_sys_socketObj,
            select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                   Field(&::timeval::tv_sec, Eq(upnplib::g_response_timeout))))
            // Consider timeout to be undefined after select() returns.
            .WillOnce(DoAll(StructSetToArg<4>(0xAA), Return(1)));
        // recv()
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
            .WillOnce(
                DoAll(StrCpyToArg<1>(received_msg),
                      Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));

        // Test Unit
        int ret_sock_read =
            sock_read(&sockinfo, buffer, sizeof(buffer), nullptr);
        EXPECT_EQ(ret_sock_read, static_cast<SSIZEP_T>(sizeof(received_msg)))
            << errStr(ret_sock_read);
        EXPECT_STREQ(buffer, received_msg);
    }
}

TEST_F(SockFTestSuite, sock_write_successful) {
    constexpr SOCKET sockfd{3};
    constexpr time_t timeout{5};
    // -1 Blocks indefinitely waiting for a socket descriptor to become ready.
    constexpr time_t no_timeout{-1};

    // select()
    EXPECT_CALL(m_sys_socketObj, // With timeout set
                select(sockfd + 1, NotNull(), NotNull(), IsNull(),
                       Field(&::timeval::tv_sec, Eq(timeout))))
        // Consider timeout to be undefined after select() returns.
        .WillOnce(DoAll(StructSetToArg<4>(0xAA), Return(1)));
    EXPECT_CALL(m_sys_socketObj, // Without timeout set
                select(sockfd + 1, NotNull(), NotNull(), IsNull(), IsNull()))
        // Consider timeout to be undefined after select() returns.
        .WillOnce(DoAll(StructSetToArg<4>(0xAA), Return(1)));
    // send()
    char sent_msg[]{"Mocked sent TCP message."};
    EXPECT_CALL(m_sys_socketObj,
                send(sockfd, sent_msg, sizeof(sent_msg),
                     Conditional(old_code, MSG_DONTROUTE | MSG_NOSIGNAL,
                                 MSG_DONTROUTE)))
        .Times(2) // With and without timeout set.
        .WillRepeatedly(Return(static_cast<SSIZEP_T>(sizeof(sent_msg))));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) {
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .Times(2)
            .WillRepeatedly(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _))
            .Times(2);
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _))
            .Times(2);
    }
#endif

    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit with timeout
    int timeoutSecs{timeout}; // Will be set to used time by the Unit.
    int ret_sock_write =
        sock_write(&sockinfo, sent_msg, sizeof(sent_msg), &timeoutSecs);

    EXPECT_EQ(ret_sock_write, static_cast<int>(sizeof(sent_msg)))
        << errStr(ret_sock_write);

    // Don't know for what next is good. I haven't found any useful
    // documentation about this "used time" and it is not used anywhere in the
    // production code. Here it is only given for compatibility. --Ingo
    EXPECT_GE(timeoutSecs, timeout - 1);

    // Test Unit without timeout
    timeoutSecs = no_timeout; // Should not be modified by the Unit
    ret_sock_write =
        sock_write(&sockinfo, sent_msg, sizeof(sent_msg), &timeoutSecs);

    EXPECT_EQ(ret_sock_write, static_cast<int>(sizeof(sent_msg)))
        << errStr(ret_sock_write);
    EXPECT_EQ(timeoutSecs, no_timeout); // Should not be modified.
}

TEST_F(SockFTestSuite, sock_write_with_connection_error) {
    constexpr SOCKET sockfd{3};

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, _, NotNull(), IsNull(), NotNull()))
        .WillOnce(SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR));

    char sent_msg[]{'\0'}; // This will not be sent.
    constexpr time_t timeout{5};
    int timeoutSecs{timeout}; // Will be set to used time by the Unit.
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_write =
        sock_write(&sockinfo, sent_msg, sizeof(sent_msg), &timeoutSecs);

    EXPECT_EQ(ret_sock_write, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_write, UPNP_E_SOCKET_ERROR);
    // On error nothing should be modified.
    EXPECT_EQ(timeoutSecs, timeout);
}

TEST_F(SockFTestSuite, sock_write_with_sending_error) {
    constexpr SOCKET sockfd{3};

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, _, NotNull(), IsNull(), NotNull()))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{'\0'}; // This will not be sent.
    EXPECT_CALL(m_sys_socketObj, send(sockfd, sent_msg, sizeof(sent_msg), _))
        .WillOnce(SetErrPtblAndReturn(ENOMEMP, SOCKET_ERROR));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
    if (old_code) {
        InSequence seq;
        // Mock getsockopt(), get old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
            .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
        // Mock setsockopt(), set new option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(1), _));
        // Mock setsockopt(), restore old option SIGPIPE
        EXPECT_CALL(m_sys_socketObj,
                    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                               PointeeVoidToConstInt(0xAA55), _));
    }
#endif

    int timeoutSecs{5}; // Will be set to used time by the Unit.
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_write =
        sock_write(&sockinfo, sent_msg, sizeof(sent_msg), &timeoutSecs);

    if (old_code) {
        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": Wrong error code 'Unknown error code(-1)', should "
                       "be 'UPNP_E_SOCKET_WRITE(-201)'.\n";
        EXPECT_EQ(ret_sock_write, -1) // Wrong!
            << errStrEx(ret_sock_write, UPNP_E_SOCKET_WRITE);

    } else {

        EXPECT_EQ(ret_sock_write, UPNP_E_SOCKET_WRITE)
            << errStrEx(ret_sock_write, UPNP_E_SOCKET_WRITE);
    }
}

TEST_F(SockFDeathTest, sock_write_with_nullptr_to_socket_info) {
    char buffer[1]{};
    int timeoutSecs{5}; // Will be set to used time by the Unit.

    if (old_code) {
        ::std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": A nullptr to a socket info structure must not segfault.\n";
        // Test Unit
        ASSERT_DEATH(sock_write(nullptr, buffer, sizeof(buffer), &timeoutSecs),
                     "");

    } else {

        // Test Unit
        ASSERT_EXIT((sock_write(nullptr, buffer, sizeof(buffer), &timeoutSecs),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_sock_write =
            sock_write(nullptr, buffer, sizeof(buffer), &timeoutSecs);
        EXPECT_EQ(ret_sock_write, UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_write, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_write_with_empty_socket_info) {
    // Configure expected system calls.
    // select() should fail with invalid socket file descriptor in sockinfo.
    if (old_code)
        EXPECT_CALL(m_sys_socketObj,
                    select(_, NotNull(), NotNull(), IsNull(), IsNull()))
            .WillOnce(SetErrPtblAndReturn(EBADFP, SOCKET_ERROR));
    else
        EXPECT_CALL(m_sys_socketObj,
                    select(_, NotNull(), NotNull(), IsNull(), IsNull()))
            .WillOnce(SetErrPtblAndReturn(EBADFP, SOCKET_ERROR));

    ::SOCKINFO sockinfo{}; // Empty socket info
    char sent_msg[1]{};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready, but not with error.

    // Test Unit
    int ret_sock_write =
        sock_write(&sockinfo, sent_msg, sizeof(sent_msg), &timeoutSecs);
    EXPECT_EQ(ret_sock_write, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_write, UPNP_E_SOCKET_ERROR);
    EXPECT_EQ(timeoutSecs, -1);
}

TEST_F(SockFTestSuite, sock_write_with_nullptr_to_buffer_but_length_given) {
    constexpr SOCKET sockfd{3};

    // Configure expected system calls.
    if (old_code) {
        // select()
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, _, NotNull(), NULL, NULL))
            .WillOnce(Return(1));
        // send()
        EXPECT_CALL(m_sys_socketObj, send(sockfd, nullptr, 1, _))
            .WillOnce(SetErrPtblAndReturn(EFAULTP, SOCKET_ERROR));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
        {           // Scope InSequence
            InSequence seq;
            // Mock getsockopt(), get old option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
                .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
            // Mock setsockopt(), set new option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                                   PointeeVoidToConstInt(1), _));
            // Mock setsockopt(), restore old option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                                   PointeeVoidToConstInt(0xAA55), _));
        } // End scope InSequence
#endif
    }

    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_write = sock_write(&sockinfo, nullptr, 1, &timeoutSecs);

    if (old_code)
        EXPECT_EQ(ret_sock_write, -1) // Wrong!
            << errStrEx(ret_sock_write, UPNP_E_SOCKET_ERROR);
    else
        EXPECT_EQ(ret_sock_write, UPNP_E_SOCKET_ERROR)
            << errStrEx(ret_sock_write, UPNP_E_SOCKET_ERROR);
}

TEST_F(SockFTestSuite, sock_write_with_valid_buffer_but_0_byte_length) {
    // This is a valid condition and indicates that there is nothing to send.
    // With 0 byte buffer length the buffer doesn't matter.
    constexpr SOCKET sockfd{sfd_base + 81};

    // Configure expected system calls.
    if (old_code) {
        // select()
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, _, NotNull(), NULL, NotNull()))
            .WillOnce(Return(1));

#ifdef SO_NOSIGPIPE // this is defined on MacOS
        {           // Scope InSequence
            InSequence seq;
            // Mock getsockopt(), get old option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        getsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, _, _))
                .WillOnce(DoAll(SetArgPtrIntValue<3>(0xAA55), Return(0)));
            // Mock setsockopt(), set new option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                                   PointeeVoidToConstInt(1), _));
            // Mock setsockopt(), restore old option SIGPIPE
            EXPECT_CALL(m_sys_socketObj,
                        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE,
                                   PointeeVoidToConstInt(0xAA55), _));
        } // End scope InSequence
#endif
    }

    char buffer[1]{};
    int timeoutSecs{5};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_write = sock_write(&sockinfo, buffer, 0, &timeoutSecs);
    EXPECT_EQ(ret_sock_write, 0)
        << "  # Should be sent number of bytes = 0"
        << " but not " << errStr(ret_sock_write) << ".";
}

TEST(SockTestSuite, sock_make_blocking_and_sock_make_no_blocking) {
#ifdef _MSC_VER
    // Windows does not offer any way to query whether a socket is currently set
    // to blocking or non-blocking. All sockets are blocking unless you
    // explicitly ioctlsocket() them with FIONBIO or hand them to either
    // WSAAsyncSelect or WSAEventSelect. The latter two functions "secretly"
    // change the socket to non-blocking. Knowing this we will do some checks.

    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, INVALID_SOCKET);

    // Now the socket is in blocking mode. Setting a blocking socket to blocking
    // is not a failure.
    EXPECT_EQ(sock_make_blocking(sock), 0);

    EXPECT_EQ(sock_make_no_blocking(sock), 0);
    EXPECT_EQ(sock_make_blocking(sock), 0);

    // But an invalid socket should fail.
    EXPECT_EQ(sock_make_blocking(INVALID_SOCKET), SOCKET_ERROR);
    EXPECT_EQ(sock_make_no_blocking(INVALID_SOCKET), SOCKET_ERROR);

    WSACleanup();
#else
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, -1);
    int flags = fcntl(sock, F_GETFL, 0);
    ASSERT_NE(flags, -1);
    EXPECT_EQ((flags & O_NONBLOCK), 0);

    EXPECT_EQ(sock_make_no_blocking(sock), 0);
    flags = fcntl(sock, F_GETFL, 0);
    EXPECT_EQ((flags & O_NONBLOCK), O_NONBLOCK);

    EXPECT_EQ(sock_make_blocking(sock), 0);
    flags = fcntl(sock, F_GETFL, 0);
    EXPECT_EQ((flags & O_NONBLOCK), 0);

    EXPECT_EQ(close(sock), 0);
#endif
}

#ifdef UPNP_ENABLE_OPEN_SSL
TEST(SockTestSuite, sock_ssl_connect) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    GTEST_FAIL()
        << "  # No tests for Open SSL connections available, must be created.";
}
#endif

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "utest/utest_main.inc"
    return gtest_return_code; // managed in gtest_main.inc
}
