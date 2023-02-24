// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-23

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <upnp.hpp>
#include <pupnp/sock.hpp>

#include "upnplib/upnptools.hpp"
#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"
#include "umock/unistd_mock.hpp"
#include "umock/sys_select_mock.hpp"
#include "umock/sys_socket_mock.hpp"

#ifndef _WIN32
#include <fcntl.h>
#endif

using ::testing::_;
using ::testing::DoAll;
using ::testing::ExitedWithCode;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArrayArgument;

using ::upnplib::errStr;
using ::upnplib::errStrEx;

namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");


// testsuite for the sock module
//==============================
#if false
TEST(SockTestSuite, sock_connect_client)
// This is for humans only to check on Unix operating systems how to 'connect()'
// to a server exactly works so we can correct mock it. Don't set '#if true'
// permanently because it connects to the real internet and may slow down this
// gtest dramatically. You may change the ip address if google.com changed its
// ip address.
{
    // Get a TCP socket
    int sockfd;
    ASSERT_NE(sockfd = ::socket(AF_INET, SOCK_STREAM, 0), -1);

    // Fill an address structure
    ::sockaddr_in saddrin{};
    saddrin.sin_family = AF_INET;
    saddrin.sin_port = htons(80);
    // This was a valid ip address from google.com
    saddrin.sin_addr.s_addr = inet_addr("172.217.18.110");

    // Connect to the server
    EXPECT_EQ(::connect(sockfd, (const sockaddr*)&saddrin,
                        sizeof(struct sockaddr_in)),
              0)
        << ::strerror(errno);

    EXPECT_EQ(::close(sockfd), 0);
}
#endif


class SockFTestSuite : public ::testing::Test {
  protected:
    // Instantiate socket object derived from the C++ interface
    Csock m_sockObj{};

    // Instantiate mock objects
    umock::Sys_socketMock m_mock_sys_socketObj;
    umock::UnistdMock m_mock_unistdObj;
    umock::Sys_selectMock m_mock_sys_selectObj;

    // Dummy socket, if we do not need a real one due to mocking
    const ::SOCKET m_socketfd{1001};

    // Provide a socket info structure
    ::SOCKINFO m_info{};
    // Point to the sockaddr_storage structure in SOCKINFO with type cast to
    // sockaddr_in.
    ::sockaddr_in* m_info_sa_in_ptr = (::sockaddr_in*)&m_info.foreign_sockaddr;

    //
    SockFTestSuite() {
        // Need to clear errno before each test because we set it sometimes for
        // mocking. The Unit under test doesn't handle it correct and we see
        // side effects between different tests with this global variable.
        errno = 0;

        m_info.socket = m_socketfd;
        m_info_sa_in_ptr->sin_family = AF_INET;
        m_info_sa_in_ptr->sin_port = htons(443);
        inet_pton(AF_INET, "192.168.24.128", &m_info_sa_in_ptr->sin_addr);

        // Set defaut return values of mocked system functions. They will fail.
        ON_CALL(m_mock_sys_selectObj, select(_, _, _, _, _))
            .WillByDefault(Return(-1));
        ON_CALL(m_mock_sys_socketObj, recv(_, _, _, _))
            .WillByDefault(Return(-1));
        ON_CALL(m_mock_sys_socketObj, send(_, _, _, _))
            .WillByDefault(Return(-1));
    }
};
typedef SockFTestSuite SockFDeathTest;


TEST_F(SockFTestSuite, sock_init_successful) {
    // Test Unit
    int ret_sock_init = m_sockObj.sock_init(&m_info, m_socketfd);
    EXPECT_EQ(ret_sock_init, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_init, UPNP_E_SUCCESS);

    EXPECT_EQ(m_info.socket, m_socketfd);
    EXPECT_EQ(m_info_sa_in_ptr->sin_family, 0);
    EXPECT_EQ(m_info_sa_in_ptr->sin_port, 0);
    EXPECT_EQ(m_info_sa_in_ptr->sin_addr.s_addr, (uint32_t)0);
}

TEST_F(SockFDeathTest, sock_init_with_no_info) {
    // Test Unit
    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Function should not segfault or abort with failed "
                     "assert().\n";
        EXPECT_DEATH(m_sockObj.sock_init(nullptr, m_socketfd), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((m_sockObj.sock_init(nullptr, m_socketfd), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_sock_init = m_sockObj.sock_init(nullptr, m_socketfd);
        EXPECT_EQ(ret_sock_init, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_sock_init, UPNP_E_INVALID_PARAM);
    }
}

TEST_F(SockFTestSuite, sock_init_with_ip_successful) {
    // Provide a sockaddr_in structure
    ::sockaddr_in foreign_sockaddr{};
    foreign_sockaddr.sin_family = AF_INET;
    foreign_sockaddr.sin_port = htons(80);
    EXPECT_EQ(inet_pton(AF_INET, "192.168.192.168", &foreign_sockaddr.sin_addr),
              1);

    // Test Unit
    int ret_sock_init_with_ip = m_sockObj.sock_init_with_ip(
        &m_info, m_socketfd, (sockaddr*)&foreign_sockaddr);
    EXPECT_EQ(ret_sock_init_with_ip, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_init_with_ip, UPNP_E_SUCCESS);

    EXPECT_EQ(m_info.socket, m_socketfd);
    EXPECT_EQ(m_info_sa_in_ptr->sin_family, AF_INET);
    EXPECT_EQ(m_info_sa_in_ptr->sin_port, htons(80));
    EXPECT_EQ(m_info_sa_in_ptr->sin_addr.s_addr,
              foreign_sockaddr.sin_addr.s_addr);
}

TEST_F(SockFDeathTest, sock_init_with_ip_but_no_ip) {
    // Error condition with no info structure is tested with
    // sock_init_with_no_info.

    // Test Unit
    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Function should not segfault.\n";
        EXPECT_DEATH(m_sockObj.sock_init_with_ip(&m_info, m_socketfd, nullptr),
                     ".*");
    } else {

        // This expects NO segfault.
        ASSERT_EXIT((m_sockObj.sock_init_with_ip(&m_info, m_socketfd, nullptr),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_sock_init_with_ip =
            m_sockObj.sock_init_with_ip(&m_info, m_socketfd, nullptr);
        EXPECT_EQ(ret_sock_init_with_ip, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_sock_init_with_ip, UPNP_E_INVALID_PARAM);
    }
}

TEST_F(SockFTestSuite, sock_destroy_successful) {
    // shutdown is successful
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close is successful
    umock::Unistd unistd_injectObj(&m_mock_unistdObj);
    EXPECT_CALL(m_mock_unistdObj, CLOSE_SOCKET_P(m_socketfd))
        .WillOnce(Return(0));

    // Test Unit
    int ret_sock_destroy =
        m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH);
    EXPECT_EQ(ret_sock_destroy, UPNP_E_SUCCESS)
        << errStrEx(ret_sock_destroy, UPNP_E_SUCCESS);
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_ok_close_fails_not_0) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // shutdown is successful
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close fails on _WIN32 with positive error number
    umock::Unistd unistd_injectObj(&m_mock_unistdObj);
    EXPECT_CALL(m_mock_unistdObj, CLOSE_SOCKET_P(m_socketfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Process the Unit
    int returned;
    if (old_code) {
        ::std::cout << "  BUG! Successful socket shutdown but close != 0 "
                       "should fail.\n";
        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);

    } else {

        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_fails_close_ok) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // shutdown fails
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(-1));
    // close is successful
    umock::Unistd unistd_injectObj(&m_mock_unistdObj);
    EXPECT_CALL(m_mock_unistdObj, CLOSE_SOCKET_P(m_socketfd))
        .WillOnce(Return(0));

    // Process the Unit
    int returned;
    errno = 1; // 'Operation not permitted'

    if (old_code) {
        ::std::cout << "  BUG! Failing socket shutdown with successful close "
                       "should fail.\n";
        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);

    } else {

        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_destroy_inval_fd_shutdown_fails_close_fails_not_0) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // shutdown fails
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(-1));
    // close fails on _WIN32 with positive error number
    umock::Unistd unistd_injectObj(&m_mock_unistdObj);
    EXPECT_CALL(m_mock_unistdObj, CLOSE_SOCKET_P(m_socketfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Process the Unit
    int returned;

    if (old_code) {
        ::std::cout
            << "  BUG! Failing socket shutdown and close != 0 should fail.\n";
        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SUCCESS)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);

    } else {

        EXPECT_EQ(returned =
                      m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
                  UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_read_no_timeout) {
    // Configure expected system calls that will return a received message.
    // select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, NotNull(), _, NULL, NULL))
        .WillOnce(Return(1));
    // recv()
    char received_msg[]{"Mocked received TCP message no timeout."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(
                            received_msg, received_msg + sizeof(received_msg)),
                        Return((SSIZEP_T)sizeof(received_msg))));

    // Test Unit
    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&m_info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, (int)sizeof(received_msg)) << errStr(returned);
    EXPECT_STREQ(buffer, received_msg);
}

TEST_F(SockFTestSuite, sock_read_within_timeout) {
    // Configure expected system calls that will return a received message.
    // select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(1));
    // recv()
    char received_msg[]{"Mocked received TCP message within timeout."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(
                            received_msg, received_msg + sizeof(received_msg)),
                        Return((SSIZEP_T)sizeof(received_msg))));

    // Process the Unit
    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{10};
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&m_info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, (int)sizeof(received_msg)) << errStr(returned);
    EXPECT_STREQ(buffer, received_msg);
}

TEST_F(SockFTestSuite, sock_read_with_connection_error) {
    // Configure expected system calls.
    // select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(-1));
    // recv()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(_, _, _, _)).Times(0);

    // Process the Unit
    char buffer[1]{};
    int timeoutSecs{10};
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&m_info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    EXPECT_STREQ(buffer, "");
}

TEST_F(SockFTestSuite, sock_read_signal_catched) {
    // A signal like ^C should not interrupt reading. When catching it, reading
    // is restarted. So we expect in this test that 'select()' is called
    // two times.
    ::std::cout << "  BUG! Must be a fix in the Unit by setting 'errno = 0;'! "
                   "(Search 'BUG!')\n";

    // Configure expected system calls. select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(-1)) // Signal catched
        .WillOnce(Return(1)); // Message received
    // recv()
    char received_msg[]{"Mocked received TCP message after signal catched."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(
                            received_msg, received_msg + sizeof(received_msg)),
                        Return((SSIZEP_T)sizeof(received_msg))));

    // Process the Unit
    char buffer[sizeof(received_msg)]{};
    int timeoutSecs{10};
    errno = EINTR; // this indicates that a signal is catched.
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&m_info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, (int)sizeof(received_msg)) << errStr(returned);
    EXPECT_STREQ(buffer, received_msg);
}

TEST_F(SockFTestSuite, sock_read_with_receiving_error) {
    // Configure expected system calls that will return a received message.
    // select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, NotNull(), _, NULL, NotNull()))
        .WillOnce(Return(1));
    // recv()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), _, _))
        .WillOnce(Return(-1));

    // Process the Unit
    char buffer[1]{};
    int timeoutSecs{10};
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&m_info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    EXPECT_STREQ(buffer, "");
}

TEST_F(SockFTestSuite, sock_read_with_invalid_pointer_to_socket_info) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "  BUG! A nullptr to a socket info structure must not "
                       "segfault.\n";

    } else {

        // Configure expected system calls should never called.
        // select()
        umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
        EXPECT_CALL(m_mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
        // recv()
        umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
        EXPECT_CALL(m_mock_sys_socketObj, recv(_, _, _, _)).Times(0);

        // Process the Unit
        int returned{UPNP_E_INTERNAL_ERROR};
        char buffer[1]{};
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        ASSERT_EXIT((returned = m_sockObj.sock_read(
                         nullptr, buffer, sizeof(buffer), &timeoutSecs),
                     exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  # A nullptr to a socket info structure must not segfault.";
        EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
            << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    }
}

TEST_F(SockFTestSuite, sock_read_with_empty_socket_info) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls that will return a received message.
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    if (old_code) {
        // select()
        ::std::cout << "  BUG! System function 'select()' must not be called. "
                       "Without timeout it may hang.\n";
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(_, NotNull(), _, NULL, NotNull()))
            .WillOnce(Return(-1));

    } else {

        ::std::cout
            << "  # System function 'select()' must not be called. Without "
               "timeout it may hang.\n";
        EXPECT_CALL(m_mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    }
    // recv()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, recv(_, _, _, _)).Times(0);

    // Process the Unit
    ::SOCKINFO info{}; // Empty socket info

    char buffer[8]{};
    int timeoutSecs{10};
    // Also on _WIN32 select() returns SOCKET_ERROR (-1).
    int returned =
        m_sockObj.sock_read(&info, buffer, sizeof(buffer), &timeoutSecs);
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);
    EXPECT_STREQ(buffer, "");
}

TEST_F(SockFTestSuite, sock_read_with_nullptr_to_buffer_0_byte_length)
// This is a valid call and indicates that there should nothing received. With 0
// byte buffer length the buffer doesn't matter and may be not available at all.
{
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    if (old_code) {
        // Configure expected system calls should never called.
        // select()
        ::std::cout
            << "  OPT: It is not needed to call system function 'select()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, NotNull(), _, NULL, NULL))
            .WillOnce(Return(1));

        // recv()
        ::std::cout
            << "  OPT: It is not needed to call system function 'recv()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, _, _, _))
            .WillOnce(Return(-1));

        // Process the Unit
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        errno = EFAULT;      // The receive buffer pointer(s) point outside the
                             // process's address space.
        int returned = m_sockObj.sock_read(&m_info, nullptr, 0, &timeoutSecs);
        EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR);
        ::std::cout << "  BUG! Should be received number of bytes = 0"
                    << " but not " << errStr(returned) << ".\n";

    } else {

        // Configure expected system calls should never called.
        // select()
        ::std::cout
            << "  OPT: It is not needed to call system function 'select()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, NotNull(), _, NULL, NULL))
            .Times(0);

        // recv()
        ::std::cout
            << "  OPT: It is not needed to call system function 'recv()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, _, _, _)).Times(0);

        // Process the Unit
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        errno = EFAULT;      // The receive buffer pointer(s) point outside the
                             // process's address space.
        int returned = m_sockObj.sock_read(&m_info, nullptr, 0, &timeoutSecs);
        EXPECT_EQ(returned, 0) << "  # Should be received number of bytes = 0"
                               << " but not " << errStr(returned) << ".";
    }
}

TEST_F(SockFTestSuite, sock_read_with_valid_buffer_but_0_byte_length)
// This is a valid call and indicates that there should nothing received. With 0
// byte buffer length the buffer doesn't matter.
{
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    if (old_code) {
        // Configure expected system calls that will return a received message.
        // select()
        ::std::cout
            << "  OPT: It is not needed to call system function 'select()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, NotNull(), _, NULL, NULL))
            .WillOnce(Return(1));

        // recv()
        ::std::cout
            << "  OPT: It is not needed to call system function 'recv()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), 0, _))
            .WillOnce(Return(-1));

        // Process the Unit
        char buffer[1]{'\0'};
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        // Also on _WIN32 select() returns SOCKET_ERROR (-1).
        int returned = m_sockObj.sock_read(&m_info, buffer, 0, &timeoutSecs);
        EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR);
        ::std::cout << "  BUG! Should be received number of bytes = 0"
                    << " but not " << errStr(returned) << ".\n";
        EXPECT_STREQ(buffer, "");

    } else {

        // Configure expected system calls that will return a received message.
        // select()
        ::std::cout
            << "  OPT: It is not needed to call system function 'select()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, NotNull(), _, NULL, NULL))
            .Times(0);

        // recv()
        ::std::cout
            << "  OPT: It is not needed to call system function 'recv()' "
               "in this case.\n";
        EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), 0, _))
            .Times(0);

        // Process the Unit
        char buffer[1]{'\0'};
        int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                             // descriptor to become ready.
        // Also on _WIN32 select() returns SOCKET_ERROR (-1).
        int returned = m_sockObj.sock_read(&m_info, buffer, 0, &timeoutSecs);
        EXPECT_EQ(returned, 0) << "  # Should be received number of bytes = 0"
                               << " but not " << errStr(returned) << ".";
        EXPECT_STREQ(buffer, "");
    }
}

TEST_F(SockFTestSuite, sock_read_with_invalid_pointer_to_timeout_value)
// The underlaying system call 'select()' accepts NULL as pointer to the timeout
// value to indicate not using a timeout and wait indefinitely. So it should
// also be possible to call the Unit with a nullptr for that.
{
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout
            << "  BUG! A nullptr to the timeout value must not segfault.\n";

    } else {

        // Configure expected system calls that will return a received message.
        // select()
        umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, NotNull(), _, NULL, NULL))
            .WillOnce(Return(1));
        // recv()
        char received_msg[]{
            "Mocked received TCP message with nullptr to timeout value."};
        umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
        EXPECT_CALL(m_mock_sys_socketObj, recv(m_socketfd, NotNull(), _, _))
            .WillOnce(
                DoAll(SetArrayArgument<1>(received_msg,
                                          received_msg + sizeof(received_msg)),
                      Return((SSIZEP_T)sizeof(received_msg))));

        // Process the Unit
        int returned{UPNP_E_INTERNAL_ERROR};
        char buffer[sizeof(received_msg)]{};
        // Also on _WIN32 select() returns SOCKET_ERROR (-1).
        ASSERT_EXIT((returned = m_sockObj.sock_read(&m_info, buffer,
                                                    sizeof(buffer), nullptr),
                     exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  # A nullptr to the timeout value must not segfault.";
        EXPECT_EQ(returned, (int)sizeof(received_msg)) << errStr(returned);
        EXPECT_STREQ(buffer, received_msg);
    }
}

TEST_F(SockFTestSuite, sock_write_no_timeout) {
    // select()
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NULL))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message no timeout."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
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
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message within timeout."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
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
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // send()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, send(_, _, _, _)).Times(0);

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
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    EXPECT_CALL(m_mock_sys_selectObj,
                select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // send()
    char sent_msg[]{"Mocked sent TCP message within timeout."};
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj,
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
        umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
        EXPECT_CALL(m_mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
        // send()
        umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
        EXPECT_CALL(m_mock_sys_socketObj, send(_, _, _, _)).Times(0);

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
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    if (old_code) {
        // select()
        ::std::cout << "  BUG! System function 'select()' must not be called. "
                       "Without timeout it may hang.\n";
        EXPECT_CALL(m_mock_sys_selectObj, select(_, NotNull(), _, NULL, NULL))
            .WillOnce(Return(-1));

    } else {

        // select()
        ::std::cout
            << "  # System function 'select()' must not be called. Without "
               "timeout it may hang.\n";
        EXPECT_CALL(m_mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    }

    // send()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, send(_, _, _, _)).Times(0);

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

TEST_F(SockFTestSuite, sock_write_with_nullptr_to_buffer_0_byte_length)
// This is a valid call and indicates that there is nothing to send. With 0 byte
// buffer length the buffer doesn't matter and may also be not available at all.
{
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls.
    // select()
    ::std::cout << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";

    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    if (old_code) {
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NULL))
            .WillOnce(Return(1));

    } else {

        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NULL))
            .Times(0);
    }

    // send()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, send(m_socketfd, _, _, _)).Times(0);

    // Process the Unit
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket
                         // descriptor to become ready.
    int returned = m_sockObj.sock_write(&m_info, nullptr, 0, &timeoutSecs);
    EXPECT_EQ(returned, 0) << "  # Should be sent number of bytes = 0"
                           << " but not " << errStr(returned) << ".";
}

TEST_F(SockFTestSuite, sock_write_with_valid_buffer_but_0_byte_length)
// This is a valid call and indicates that there is nothing to send. With 0 byte
// buffer length the buffer doesn't matter.
{
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Configure expected system calls.
    umock::Sys_select sys_select_injectObj(&m_mock_sys_selectObj);
    if (old_code) {
        // select()
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
            .WillOnce(Return(1));

    } else {

        // select()
        EXPECT_CALL(m_mock_sys_selectObj,
                    select(m_socketfd + 1, _, NotNull(), NULL, NotNull()))
            .Times(0);
    }

    ::std::cout << "  OPT: It is not needed to call system function 'select()' "
                   "in this case.\n";

    // send()
    umock::Sys_socket sys_socket_injectObj(&m_mock_sys_socketObj);
    EXPECT_CALL(m_mock_sys_socketObj, send(m_socketfd, _, _, _)).Times(0);

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
    EXPECT_EQ(sock_make_blocking((SOCKET)-2), -1);
    EXPECT_EQ(sock_make_no_blocking((SOCKET)-2), -1);

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

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
