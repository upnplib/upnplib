// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-30

// Helpful link for ip address structures:
// https://stackoverflow.com/q/76548580/5014688

#include <upnp.hpp>

#include <upnplib/general.hpp>
#include <upnplib/upnptools.hpp> // For errStr??
#include <upnplib/sockaddr.hpp>
#include <upnplib/socket.hpp>

#include <pupnp/upnpdebug.hpp>

#include <utest/utest.hpp>

#include <umock/unistd_mock.hpp>
#include <umock/sys_socket_mock.hpp>

#define sockaddr_storage upnplib::sockaddr_t
#include <sock.hpp>

namespace utest {

using ::testing::_;
using ::testing::DoAll;
using ::testing::ExitedWithCode;
using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;
using ::testing::StrictMock;

using ::utest::StrCpyToArg;

using ::upnplib::errStr;
using ::upnplib::errStrEx;
using ::upnplib::g_dbug;
using ::upnplib::SSockaddr;

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
        .WillOnce(SetErrnoAndReturn(ENOTCONNP, SOCKET_ERROR))
        // Second call: shutdown error on connected connection.
        .WillOnce(SetErrnoAndReturn(EBADFP, SOCKET_ERROR));
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
#ifdef _MSC_VER
        ::WSASetLastError(ENOTCONNP); // Instead of mocking
#endif
        EXPECT_EQ(ret_sock_destroy =
                      sock_destroy(&sockinfo, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(ret_sock_destroy, UPNP_E_SUCCESS);

        // Second call: shutdown error on connected connection.
        sock_init(&sockinfo, sockfd);
#ifdef _MSC_VER
        ::WSASetLastError(EBADFP); // Instead of mocking
#endif
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
        .WillOnce(Return(SOCKET_ERROR));
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

TEST_F(SockFTestSuite, sock_read_within_timeout_successful) {
    constexpr SOCKET sockfd{sfd_base + 71};

    // Configure expected system calls that will return a received message.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(1));
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

    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{10};
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

TEST_F(SockFTestSuite, sock_read_no_timeout_successful) {
    constexpr SOCKET sockfd{sfd_base + 70};

    // Configure expected system calls that will return a received message.
    // select()
    EXPECT_CALL(m_sys_socketObj, select(sockfd + 1, NotNull(), _, NULL, NULL))
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
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
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
    constexpr SOCKET sockfd{sfd_base + 72};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);
    char buffer[1]{'\xAA'};
    int timeoutSecs{10};

    // Configure expected system calls.
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), _, NULL, NotNull()))
#ifndef _MSC_VER
        .WillOnce(SetErrnoAndReturn(EBADFP, SOCKET_ERROR));
#else
        .WillOnce(SetErrnoAndReturn(0, SOCKET_ERROR));
    ::WSASetLastError(EBADFP); // Set instead of mocking, just before testing
#endif
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
    constexpr SOCKET sockfd{sfd_base + 73};
    constexpr char received_msg[]{
        "Mocked received TCP message after signal catched."};
    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{10};
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

#ifdef _MSC_VER
    if (old_code) {
        // select
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), _, NULL, NotNull()))
            .WillOnce(Return(SOCKET_ERROR));

        // Test Unit
        ::WSASetLastError(EINTRP); // Instead of mocking
        int ret_sock_read =
            sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

        ::std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                    << ": Unit must not fail reading network data on win32 due "
                       "to wrong error checking.\n";
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR) // Wrong!
            << errStr(ret_sock_read);
        EXPECT_STREQ(buffer, ""); // Wrong!

    } else {

        // select
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), _, NULL, NotNull()))
            .WillOnce(Return(SOCKET_ERROR)) // Signal catched
            .WillOnce(Return(1));           // Message received
        // recv()
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
            .WillOnce(
                DoAll(StrCpyToArg<1>(received_msg),
                      Return(static_cast<SSIZEP_T>(sizeof(received_msg)))));

        // Test Unit
        ::WSASetLastError(EINTRP); // Instead of mocking
        int ret_sock_read =
            sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);

        EXPECT_EQ(ret_sock_read, sizeof(received_msg)) << errStr(ret_sock_read);
        EXPECT_STREQ(buffer, received_msg);
    }

#else // _MSC_VER

    // select
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(SetErrnoAndReturn(EINTRP, SOCKET_ERROR)) // Signal catched
        .WillOnce(Return(1));                              // Message received
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
    constexpr SOCKET sockfd{sfd_base + 74};

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(sockfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(1));
    // recv()
    EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), _, _))
        .WillOnce(Return(SOCKET_ERROR));

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
    int timeoutSecs{10};
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

#if 0
TEST(SockTestSuite, sock_write_read_successful) {
    constexpr char received_msg[]{"Received message"};
    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{10};
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    // Test Unit
    int ret_sock_read =
        sock_read(&sockinfo, buffer, sizeof(buffer), &timeoutSecs);
}
#endif

// TODO: needs to be tested with real syscalls.
TEST_F(SockFTestSuite, sock_read_with_nullptr_to_buffer_or_0_byte_length) {
    // This is a valid call and indicates that there should nothing received.
    // With no pointer to a buffer, the buffer length doesn't matter.
    // With buffer pointer but 0 byte length, the buffer doesn't mätter.
    constexpr SOCKET sockfd{sfd_base + 75};
    int timeoutSecs{10}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    ::SOCKINFO sockinfo;
    sock_init(&sockinfo, sockfd);

    if (old_code) {
        // Configure expected system calls should never called.
        // select()
        if (g_dbug)
            ::std::cout
                << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), NotNull(), NULL, NotNull()))
            .WillOnce(Return(1))
            .WillOnce(Return(1));

        // recv()
        if (g_dbug)
            ::std::cout
                << "  OPT: It is not needed to call system function 'recv()' "
                   "in this case.\n";
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, _, _, _))
            .WillOnce(Return(SOCKET_ERROR));
        EXPECT_CALL(m_sys_socketObj, recv(sockfd, NotNull(), 0, _))
            .WillOnce(Return(SOCKET_ERROR));

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
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR); // Wrong!
        ::std::cout
            << CYEL "[ FIX      ] " CRES << __LINE__
            << ": nullptr to buffer should receive number of bytes = 0 but not "
            << errStr(ret_sock_read) << ".\n";

        char buffer[]{""};
        ret_sock_read = sock_read(&sockinfo, buffer, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, UPNP_E_SOCKET_ERROR); // Wrong!
        EXPECT_STREQ(buffer, "");
        ::std::cout << CYEL "[ FIX      ] " CRES << __LINE__
                    << ": 0 byte buffer length should receive number of bytes "
                       "= 0 but not "
                    << errStr(ret_sock_read) << ".\n";

    } else {

        // Test Unit
        int ret_sock_read = sock_read(&sockinfo, nullptr, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, 0)
            << "  # Should receive number of bytes = 0"
            << " but not " << errStr(ret_sock_read) << ".";

        char buffer[]{""};
        ret_sock_read = sock_read(&sockinfo, buffer, 0, &timeoutSecs);
        EXPECT_EQ(ret_sock_read, 0)
            << "  # Should receive number of bytes = 0"
            << " but not " << errStr(ret_sock_read) << ".";
        EXPECT_STREQ(buffer, "");
    }
}

TEST_F(SockFDeathTest, sock_read_with_nullptr_to_timeout_value) {
    // Specifying a nullptr for the timeout value results in using the default
    // response timeout.
    constexpr SOCKET sockfd{sfd_base + 76};
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
        EXPECT_CALL(m_sys_socketObj,
                    select(sockfd + 1, NotNull(), _, NULL, NotNull()))
            .WillOnce(Return(1));
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

#if 0
TEST_F(SockFTestSuite, sock_write_no_timeout) {
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NULL))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message no timeout."};
    EXPECT_CALL(m_sys_socketObj,
                send(m_socketfd, sent_msg, sizeof(sent_msg), _))
        .WillOnce(Return((SSIZEP_T)sizeof(sent_msg)));

    // Process the Unit
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    int returned =
        m_sockObj.sock_write(&m_info, sent_msg, sizeof(sent_msg), &timeoutSecs);
    EXPECT_EQ(returned, (int)sizeof(sent_msg)) << errStr(returned);
}

TEST_F(SockFTestSuite, sock_write_within_timeout) {
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message within timeout."};
    EXPECT_CALL(m_sys_socketObj,
                send(m_socketfd, sent_msg, sizeof(sent_msg), _))
        .WillOnce(Return((SSIZEP_T)sizeof(sent_msg)));

    // Process the Unit
    int timeoutSecs{10};
    int returned =
        m_sockObj.sock_write(&m_info, sent_msg, sizeof(sent_msg), &timeoutSecs);
    EXPECT_EQ(returned, (int)sizeof(sent_msg)) << errStr(returned);
}

TEST_F(SockFTestSuite, sock_write_with_connection_error) {
    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // send()
    EXPECT_CALL(m_sys_socketObj, send(_, _, _, _)).Times(0);

    // Process the Unit
    char sent_msg[]{"Mocked sent TCP message within timeout."};
    int timeoutSecs{10}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    int returned =
        m_sockObj.sock_write(&m_info, sent_msg, sizeof(sent_msg), &timeoutSecs);
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);
}

TEST_F(SockFTestSuite, sock_write_with_sending_error) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // select()
    EXPECT_CALL(m_sys_socketObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message within timeout."};
    EXPECT_CALL(m_sys_socketObj,
                send(m_socketfd, sent_msg, sizeof(sent_msg), _))
        .WillOnce(Return(-1));

    // Process the Unit
    int timeoutSecs{10};
    int returned =
        m_sockObj.sock_write(&m_info, sent_msg, sizeof(sent_msg), &timeoutSecs);

    if (old_code) {
        ::std::cout << "  BUG! Wrong error message 'Unknown error code(-1)', "
                       "should be 'UPNP_E_SOCKET_ERROR(-208)'.\n";
        EXPECT_EQ(returned, -1) << errStrEx(returned, UPNP_E_SOCKET_ERROR);

    } else {

        EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_write_with_nullptr_to_socket_info) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "  BUG! A nullptr to a socket info structure must not "
                       "segfault.\n";

    } else {

        // Configure expected system calls should never called.
        // select()
        EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);
        // send()
        EXPECT_CALL(m_sys_socketObj, send(_, _, _, _)).Times(0);

        // Process the Unit
        int returned{UPNP_E_INTERNAL_ERROR};
        char buffer[1]{};
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        ASSERT_EXIT((returned = m_sockObj.sock_write(
                         nullptr, buffer, sizeof(buffer), &timeoutSecs),
                     exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  # A nullptr to a socket info structure must not segfault.";
        EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_write_with_empty_socket_info) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls.
    if (old_code) {
        // select()
        ::std::cout << "  BUG! System function 'select()' must not be called. "
                       "Without timeout it may hang.\n";
        EXPECT_CALL(m_sys_socketObj, select(_, NotNull(), _, NULL, NULL))
            .WillOnce(Return(-1));

    } else {

        // select()
        ::std::cout
            << "  # System function 'select()' must not be called. Without "
               "timeout it may hang.\n";
        EXPECT_CALL(m_sys_socketObj, select(_, _, _, _, _)).Times(0);
    }

    // send()
    EXPECT_CALL(m_sys_socketObj, send(_, _, _, _)).Times(0);

    // Process the Unit
    ::SOCKINFO info{}; // Empty socket info

    char sent_msg[]{"Mocked sent TCP message with empty socket info."};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_write(&info, sent_msg, sizeof(sent_msg), &timeoutSecs);
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);
}

TEST_F(SockFTestSuite, sock_write_with_nullptr_to_buffer_0_byte_length) {
    // This is a valid call and indicates that there is nothing to send. With 0
    // byte buffer length the buffer doesn't matter and may also be not
    // available at all.
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls.
    // select()
    ::std::cout << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";

    if (old_code) {
        EXPECT_CALL(m_sys_socketObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NULL))
            .WillOnce(Return(1));

    } else {

        EXPECT_CALL(m_sys_socketObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NULL))
            .Times(0);
    }

    // send()
    EXPECT_CALL(m_sys_socketObj, send(m_socketfd, _, _, _)).Times(0);

    // Process the Unit
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    int returned = m_sockObj.sock_write(&m_info, nullptr, 0, &timeoutSecs);
    EXPECT_EQ(returned, 0) << "  # Should be sent number of bytes = 0"
                           << " but not " << errStr(returned) << ".";
}

TEST_F(SockFTestSuite, sock_write_with_valid_buffer_but_0_byte_length) {
    // This is a valid call and indicates that there is nothing to send. With 0
    // byte buffer length the buffer doesn't matter.
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls.
    if (old_code) {
        // select()
        EXPECT_CALL(m_sys_socketObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
            .WillOnce(Return(1));

    } else {

        // select()
        EXPECT_CALL(m_sys_socketObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
            .Times(0);
    }

    ::std::cout << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";

    // send()
    EXPECT_CALL(m_sys_socketObj, send(m_socketfd, _, _, _)).Times(0);

    // Process the Unit
    char buffer[1]{'\0'};
    int timeoutSecs{10};
    int returned = m_sockObj.sock_write(&m_info, buffer, 0, &timeoutSecs);
    EXPECT_EQ(returned, 0) << "  # Should be sent number of bytes = 0"
                           << " but not " << errStr(returned) << ".";
}

TEST(SockTestSuite, sock_make_blocking_and_sock_make_no_blocking) {
#ifdef _WIN32
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
    EXPECT_EQ(sock_make_blocking((SOCKET)-2), SOCKET_ERROR);
    EXPECT_EQ(sock_make_no_blocking((SOCKET)-2), SOCKET_ERROR);

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
#endif

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "utest/utest_main.inc"
    return gtest_return_code; // managed in gtest_main.inc
}
