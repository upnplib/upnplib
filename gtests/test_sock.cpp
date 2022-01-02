// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-07

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include "port.hpp"
#include "gmock/gmock.h"
#include "custom_gtest_tools_all.hpp"
#include "upnpmock/sys_socket.hpp"
#include "upnpmock/sys_select.hpp"
#include "upnpmock/unistd.hpp"

#include "genlib/net/sock.cpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;
using ::testing::SetArrayArgument;

namespace upnp {

// Interface for the sock module
// =============================
// clang-format off

class Isock {
  public:
    virtual ~Isock() {}

    virtual int sock_init(
        SOCKINFO* info, SOCKET sockfd) = 0;
    virtual int sock_init_with_ip(
        SOCKINFO* info, SOCKET sockfd, struct sockaddr* foreign_sockaddr) = 0;
#ifdef UPNP_ENABLE_OPEN_SSL
    virtual int sock_ssl_connect(
        SOCKINFO* info) = 0;
#endif
    virtual int sock_destroy(
        SOCKINFO* info, int ShutdownMethod) = 0;
    virtual int sock_read(
        SOCKINFO* info, char* buffer, size_t bufsize, int* timeoutSecs) = 0;
    virtual int sock_write(
        SOCKINFO* info, const char* buffer, size_t bufsize, int* timeoutSecs) = 0;
    virtual int sock_make_blocking(
        SOCKET sock) = 0;
    virtual int sock_make_no_blocking(
        SOCKET sock) = 0;
};

class Csock : Isock {
  public:
    virtual ~Csock() override {}

    int sock_init(SOCKINFO* info, SOCKET sockfd) override {
        return ::sock_init(info, sockfd); }
    int sock_init_with_ip( SOCKINFO* info, SOCKET sockfd, struct sockaddr* foreign_sockaddr) override {
        return ::sock_init_with_ip(info, sockfd, foreign_sockaddr); }
#ifdef UPNP_ENABLE_OPEN_SSL
    int sock_ssl_connect( SOCKINFO* info) override {
        return ::sock_ssl_connect(info); }
#endif
    int sock_destroy(SOCKINFO* info, int ShutdownMethod) override {
        return ::sock_destroy(info, ShutdownMethod); }
    int sock_read(SOCKINFO* info, char* buffer, size_t bufsize, int* timeoutSecs) override {
        return ::sock_read(info, buffer, bufsize, timeoutSecs); }
    int sock_write(SOCKINFO* info, const char* buffer, size_t bufsize, int* timeoutSecs) override {
        return ::sock_write(info, buffer, bufsize, timeoutSecs); }
    int sock_make_blocking(SOCKET sock) override {
        return ::sock_make_blocking(sock); }
    int sock_make_no_blocking(SOCKET sock) override {
        return ::sock_make_no_blocking(sock); }
};
// clang-format on

//
// Mocked system calls
// ===================
// See the respective include files in upnp/include/upnpmock/

class Mock_sys_socket : public Bsys_socket {
    // Class to mock the free system functions.
    Bsys_socket* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_socket() {
        m_oldptr = sys_socket_h;
        sys_socket_h = this;
    }
    virtual ~Mock_sys_socket() override { sys_socket_h = m_oldptr; }

    MOCK_METHOD(UPNP_SIZE_T_INT, recv,
                (int sockfd, char* buf, size_t len, int flags), (override));
    MOCK_METHOD(UPNP_SIZE_T_INT, send,
                (int sockfd, const char* buf, size_t len, int flags),
                (override));
    MOCK_METHOD(int, shutdown, (int sockfd, int how), (override));
};

class Mock_sys_select : public Bsys_select {
    // Class to mock the free system functions.
    Bsys_select* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_select() {
        m_oldptr = sys_select_h;
        sys_select_h = this;
    }
    virtual ~Mock_sys_select() override { sys_select_h = m_oldptr; }

    MOCK_METHOD(int, select,
                (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
                 struct timeval* timeout),
                (override));
};

class Mock_unistd : public Bunistd {
    // Class to mock the free system functions.
    Bunistd* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_unistd() {
        m_oldptr = unistd_h;
        unistd_h = this;
    }
    virtual ~Mock_unistd() override { unistd_h = m_oldptr; }

    MOCK_METHOD(int, UPNP_CLOSE_SOCKET, (UPNP_SOCKET_TYPE fd), (override));
};

//
// testsuite for the sock module
//==============================
class SockFTestSuite : public ::testing::Test {
  protected:
    // Instantiate socket object derived from the C++ interface
    Csock m_sockObj{};

    // Instantiate mock objects
    Mock_sys_socket m_mock_sys_socketObj;
    // Mock_sys_select m_mocked_sys_selectObj;
    Mock_unistd m_mock_unistdObj;

    // Dummy socket, if we do not need a real one due to mocking
    const ::SOCKET m_socketfd{147};

    // Provide a socket info structure
    ::SOCKINFO m_info{};
    // Point to the sockaddr_storage structure in SOCKINFO with type cast to
    // sockaddr_in.
    ::sockaddr_in* m_info_sa_in_ptr = (::sockaddr_in*)&m_info.foreign_sockaddr;

    //
    SockFTestSuite() {
        m_info.socket = m_socketfd;
        m_info_sa_in_ptr->sin_family = AF_INET;
        m_info_sa_in_ptr->sin_port = htons(443);
        m_info_sa_in_ptr->sin_addr.s_addr = inet_addr("192.168.24.128");
    }
};

TEST_F(SockFTestSuite, sock_init) {
    // Process the Unit
    int rc;
    EXPECT_EQ(rc = m_sockObj.sock_init(&m_info, m_socketfd), UPNP_E_SUCCESS)
        << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS << "), but not "
        << UpnpGetErrorMessage(rc) << '(' << rc << ").";
    EXPECT_EQ(m_info.socket, m_socketfd);
}

TEST_F(SockFTestSuite, sock_init_with_ip) {
    // Provide a sockaddr_in structure
    ::sockaddr_in foreign_sockaddr{};
    foreign_sockaddr.sin_family = AF_INET;
    foreign_sockaddr.sin_port = htons(80);
    foreign_sockaddr.sin_addr.s_addr = inet_addr("192.168.192.168");

    // Process the Unit
    int rc;
    EXPECT_EQ(rc = m_sockObj.sock_init_with_ip(&m_info, m_socketfd,
                                               (sockaddr*)&foreign_sockaddr),
              UPNP_E_SUCCESS)
        << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS << "), but not "
        << UpnpGetErrorMessage(rc) << '(' << rc << ").";
    EXPECT_EQ(m_info.socket, m_socketfd);
    EXPECT_EQ(m_info_sa_in_ptr->sin_port, htons(80));
    EXPECT_EQ(m_info_sa_in_ptr->sin_addr.s_addr, inet_addr("192.168.192.168"));
}

TEST_F(SockFTestSuite, sock_destroy_valid_socket_descriptor) {
    // shutdown is successful
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close is successful
    EXPECT_CALL(m_mock_unistdObj, UPNP_CLOSE_SOCKET(m_socketfd))
        .WillOnce(Return(0));

    // Process the Unit
    int rc;
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SUCCESS)
        << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS << "), but not "
        << UpnpGetErrorMessage(rc) << '(' << rc << ").";
}

TEST_F(SockFTestSuite,
       sock_destroy_invalid_fd_shutdown_ok_close_fails_not_zero) {
    // shutdown is successful
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(0));
    // close fails on _WIN32 with positive error number
    EXPECT_CALL(m_mock_unistdObj, UPNP_CLOSE_SOCKET(m_socketfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Process the Unit
    int rc;
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! Successful socket shutdown but close != 0 should fail.\n";
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SUCCESS)
#else
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SOCKET_ERROR)
#endif
        << "  # Should be UPNP_E_SOCKET_ERROR(" << UPNP_E_SOCKET_ERROR
        << "), but not " << UpnpGetErrorMessage(rc) << '(' << rc << ").";
}

TEST_F(SockFTestSuite, sock_destroy_invalid_fd_shutdown_fails_close_ok) {
    // shutdown fails
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(-1));
    // close is successful
    EXPECT_CALL(m_mock_unistdObj, UPNP_CLOSE_SOCKET(m_socketfd))
        .WillOnce(Return(0));

    // Process the Unit
    int rc;
    errno = 1; // 'Operation not permitted'
#ifdef OLD_TEST
    ::std::cout << "  BUG! Failing socket shutdown with successful close "
                   "should fail.\n";
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SUCCESS)
#else
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SOCKET_ERROR)
#endif
        << "  # Should be UPNP_E_SOCKET_ERROR(" << UPNP_E_SOCKET_ERROR
        << "), but not " << UpnpGetErrorMessage(rc) << '(' << rc << ").";
}

TEST_F(SockFTestSuite,
       sock_destroy_invalid_fd_shutdown_fails_close_fails_not_zero) {
    // shutdown fails
    EXPECT_CALL(m_mock_sys_socketObj,
                shutdown(m_socketfd, /*SHUT_RDWR*/ SD_BOTH))
        .WillOnce(Return(-1));
    // close fails on _WIN32 with positive error number
    EXPECT_CALL(m_mock_unistdObj, UPNP_CLOSE_SOCKET(m_socketfd))
        .WillOnce(Return(10093 /*WSANOTINITIALISED*/));

    // Process the Unit
    int rc;
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! Failing socket shutdown and close != 0 should fail.\n";
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SUCCESS)
#else
    EXPECT_EQ(rc = m_sockObj.sock_destroy(&m_info, /*SHUT_RDWR*/ SD_BOTH),
              UPNP_E_SOCKET_ERROR)
#endif
        << "  # Should be UPNP_E_SOCKET_ERROR(" << UPNP_E_SOCKET_ERROR
        << "), but not " << UpnpGetErrorMessage(rc) << '(' << rc << ").";
}

#ifdef UPNP_ENABLE_OPEN_SSL
TEST(SockTestSuite, sock_ssl_connect) {
    GTEST_FAIL()
        << "  # No tests for Open SSL connections available, must be created.";
}
#endif

#if false
TEST_F(SockFTestSuite, sock_read) {
    // Fill a socket info structure ...
    ::SOCKINFO info{};

    // ... with a socket
    // info.socket = ::socket(AF_INET, SOCK_STREAM, 0);
    // ASSERT_NE(info.socket, -1);
    info.socket = m_socketfd;
    // ... and point to the sockaddr_storage structure in SOCKINFO with type
    // cast to sockaddr_in.
    sockaddr_in* foreign_sa_in_ptr = (sockaddr_in*)&info.foreign_sockaddr;
    // ... and set ipv4 (AF_INET) values in sockaddr_storage (type casted).
    foreign_sa_in_ptr->sin_family = AF_INET;
    foreign_sa_in_ptr->sin_port = htons(2399);
    ASSERT_EQ(::inet_pton(AF_INET, "127.0.0.1", &foreign_sa_in_ptr->sin_addr),
              1);

    // Provide other needed arguments
    char buffer[64]{};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket descriptor
                         // to become ready.

    // It seems mocking of select isn't needed with a real socket
    // EXPECT_CALL(m_mocked_sys_selectObj, select(_, _, _, _, _))
    //     .WillOnce(Return(1));

    char mock_receive[]{"Mocked received TCP message."};
    EXPECT_CALL(m_mock_sys_socketObj, recv(_, NotNull(), _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(
                            mock_receive, mock_receive + sizeof(mock_receive)),
                        Return(sizeof(mock_receive))));

    // Process the Unit
    Csock sockObj{};
    int rc;
    EXPECT_EQ(
        rc = sockObj.sock_read(&info, buffer, sizeof(buffer), &timeoutSecs),
        sizeof(mock_receive))
        << UpnpGetErrorMessage(rc) << '(' << rc << ')';
    EXPECT_STREQ(buffer, mock_receive);

    EXPECT_EQ(rc = sockObj.sock_destroy(&info, SD_BOTH), UPNP_E_SUCCESS)
        << UpnpGetErrorMessage(rc) << '(' << rc << ')';
}

TEST_F(SockFTestSuite, sock_write) {
    // Fill a socket info structure ...
    ::SOCKINFO info{};

    // ... with a socket
    info.socket = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(info.socket, -1);
    // ... and point to the sockaddr_storage structure in SOCKINFO with type
    // cast to sockaddr_in.
    sockaddr_in* foreign_sa_in_ptr = (sockaddr_in*)&info.foreign_sockaddr;
    // ... and set ipv4 (AF_INET) values in sockaddr_storage (type casted).
    foreign_sa_in_ptr->sin_family = AF_INET;
    foreign_sa_in_ptr->sin_port = htons(2399);
    ASSERT_EQ(::inet_pton(AF_INET, "127.0.0.1", &foreign_sa_in_ptr->sin_addr),
              1);

    // Provide other needed arguments
    char buffer[]{"Mocked sent TCP message."};
    int timeoutSecs{-1}; // -1 Blocks indefinitely waiting for a socket descriptor
                         // to become ready.

    // It seems mocking of select isn't needed with a real socket
    // EXPECT_CALL(m_mocked_sys_selectObj, select(_, _, _, _, _))
    //     .WillOnce(Return(1));

    EXPECT_CALL(m_mock_sys_socketObj, send(_, NotNull(), _, _))
        .WillOnce(Return(sizeof(buffer)));

    // Process the Unit
    Csock sockObj{};
    int rc;
    EXPECT_EQ(
        rc = sockObj.sock_write(&info, buffer, sizeof(buffer), &timeoutSecs),
        sizeof(buffer))
        << UpnpGetErrorMessage(rc) << '(' << rc << ')';

    EXPECT_EQ(rc = sockObj.sock_destroy(&info, SD_BOTH), UPNP_E_SUCCESS)
        << UpnpGetErrorMessage(rc) << '(' << rc << ')';
}
#endif

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
