// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-25

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <sock.hpp>

#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/gtest.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif

#include <gmock/gmock.h>

#include <cstring>

using upnplib::testing::CaptureStdOutErr;

extern SSL_CTX* gSslCtx;


namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");


class SockFTestSuite : public ::testing::Test {
#ifdef _WIN32
    // Initialize and cleanup Windows sochets
  protected:
    SockFTestSuite() {
        WSADATA wsaData;
        int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc != NO_ERROR) {
            throw std::runtime_error(
                std::string("Failed to start Windows sockets (WSAStartup)."));
        }
    }

    ~SockFTestSuite() override { WSACleanup(); }
#endif
};


TEST_F(SockFTestSuite, libssl_connection_error_handling) {
    // This test shows how to use libssl to initialize a connection and handle
    // some errors. It does not test a Unit. For discussion of the SIGPIPE
    // signal abort have a look at: https://stackoverflow.com/q/108183/5014688
    // and https://stackoverflow.com/a/5283463/5014688. Signal handlers are
    // process global so we should not use signal() since it's extremely bad
    // behavior for a library to alter the caller's signal handlers..

    // Provide a context structure.
    SSL_CTX* ssl_ctx{};

    // Create connection context
    ssl_ctx = SSL_CTX_new(TLS_method());
    ASSERT_NE(ssl_ctx, nullptr);

    // Create structure for a connection
    SSL* ssl = SSL_new(ssl_ctx);
    if (ssl == nullptr)
        SSL_CTX_free(ssl_ctx);
    ASSERT_NE(ssl, nullptr);

    // Associate a socket to a ssl connection
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        SSL_free(ssl);
        SSL_CTX_free(ssl_ctx);
    }
    ASSERT_NE(sockfd, INVALID_SOCKET);
    // Due to openssl man page, type cast is no problem.
    EXPECT_EQ(SSL_set_fd(ssl, (int)sockfd), 1);

    // Connect to remote peer
    // If we have a broken connection, as given here, a try to connect will
    // silently abort the program with a SIGPIPE signal. This can be ignored
    // with the following.
    // BUT IT SHOULD NOT BE USED IN A LIBRARY! See note above.
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN); // SIGPIPE not defined on MS Windows
#endif
    EXPECT_EQ(SSL_connect(ssl), -1); // error
    int err_no = errno;
#ifndef _WIN32
    signal(SIGPIPE, SIG_DFL); // default signal handling
#endif

    // errno reports error
    switch (err_no) {
#ifdef _WIN32
    case 0: // no error reported on MS Windows
        break;
#endif
    case 32: // "Broken pipe" on Ubuntu
        break;
    case 57: // "Socket is not connected" on MacOS
        break;
    default:
        EXPECT_EQ(err_no, 32);
        EXPECT_EQ(err_no, 57);
        std::cout << "errno " << err_no << ": \"" << std::strerror(err_no)
                  << "\"\n";
        break;
    }

    // There are no errors reported from ssl
    CaptureStdOutErr stdErr(STDERR_FILENO); // or STDOUT_FILENO
    stdErr.start();
    ERR_print_errors_fp(stderr);
    EXPECT_EQ(stdErr.get(), "");

    // There is no connection established. We must not use
    // EXPECT_EQ(SSL_shutdown(ssl), -1);

    EXPECT_EQ(CLOSE_SOCKET_P(sockfd), 0);
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
}

#if 0
TEST(SockTestSuite, sock_ssl_connect_signal_broken_pipe) {
    // Provide a socket info structure
    SOCKINFO info{};
    Csock sockObj;

    // Initialize the global connection structure
    gSslCtx = SSL_CTX_new(TLS_method());
    ASSERT_NE(gSslCtx, nullptr);

    ASSERT_NE(info.socket = ::socket(AF_INET, SOCK_STREAM, 0), -1);

    // Test Unit
    // This crashes silently with SIGPIPE because there is nothing on the other
    // end of the connection. The signal can be suppressed with
    // signal(SIGPIPE, SIG_IGN); See other test.
    EXPECT_DEATH(sockObj.sock_ssl_connect(&info), ".*");

    SSL_free(info.ssl);
    SSL_CTX_free(gSslCtx);
}

TEST_F(SockFTestSuite, sock_ssl_connect_suppress_signal_broken_pipe) {
    // Initialize the global connection structure
    gSslCtx = SSL_CTX_new(TLS_method());
    ASSERT_NE(gSslCtx, nullptr);

    ASSERT_NE(m_info.socket = ::socket(AF_INET, SOCK_STREAM, 0), -1);
    //m_info.socket = m_socketfd;

    // Test Unit
    // This would crash silently with SIGPIPE because there is nothing on the
    // other end of the connection. The signal is suppressed with
    signal(SIGPIPE, SIG_IGN);
    int ret_sock_ssl_connect = m_sockObj.sock_ssl_connect(&m_info);
    EXPECT_EQ(ret_sock_ssl_connect, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_ssl_connect, UPNP_E_SOCKET_ERROR);
    signal(SIGPIPE, SIG_DFL); // default signal handling

    EXPECT_NE(m_info.ssl, nullptr);

    SSL_free(m_info.ssl);
    SSL_CTX_free(gSslCtx);
}
#endif

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
