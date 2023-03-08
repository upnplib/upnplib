// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <pupnp/sock.hpp>
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS ::pupnp
#else
#define NS ::compa
#include <compa/sock.hpp>
#endif

#include <upnp.hpp>

#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/gtest.hpp>
#include "upnplib/upnptools.hpp" // for errStrEx

#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif

#include <gmock/gmock.h>

#include <cstring>

using ::testing::ExitedWithCode;

using upnplib::errStrEx;
using upnplib::testing::CaptureStdOutErr;

UPNPLIB_EXTERN SSL_CTX* gSslCtx;


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
typedef SockFTestSuite SockFDeathTest;


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
    EXPECT_EQ(SSL_connect(ssl), -1); // error, underlaying socket no connection
    int err_no = errno;
#ifndef _WIN32
    signal(SIGPIPE, SIG_DFL); // default signal handling
#endif

    // errno reports error
#ifdef _WIN32
    // 22: "Invalid argument"
    EXPECT_EQ(err_no, 22) << "  " << std::strerror(err_no) << "(" << err_no
                          << ")";
#elif __APPLE__
    // 57: "Socket is not connected"
    EXPECT_EQ(err_no, 57) << "  " << std::strerror(err_no) << "(" << err_no
                          << ")";
#else
    // 32: "Broken pipe" on Ubuntu
    EXPECT_EQ(err_no, 32) << "  " << std::strerror(err_no) << "(" << err_no
                          << ")";
#endif

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

TEST_F(SockFDeathTest, sock_ssl_connect_signal_broken_pipe) {
    // Steps as given by the Unit and expected results:
    // 1. SSL_new(): set new global SSL Context   - succeeds
    // 2. SSL_set_fd(): set socket to SSL Context - succeeds
    // 3. SSL_connect(): connect to remote node   - fails

    // Setup needed environment that is:
    // a) a socket info structure
    SOCKINFO info{};
    // b) a socket in the info structure that is expected to have a valid
    //    connection to a remote server. Here it hasn't.
    ASSERT_NE(info.socket = ::socket(AF_INET, SOCK_STREAM, 0), INVALID_SOCKET);
    // c) initialize the global SSL Context
    gSslCtx = SSL_CTX_new(TLS_method());
    if (gSslCtx == nullptr) {
        EXPECT_EQ(CLOSE_SOCKET_P(info.socket), 0);
        // This always fails the test because gSslCtx == nullptr
        ASSERT_NE(gSslCtx, nullptr);
    }
    // d) provide a C++ interface object to call the Unit
    NS::Csock sockObj;

    // Test Unit
#if !defined __APPLE__ && !defined _WIN32
    if (old_code) {

        // This crashes silently with SIGPIPE because there is nothing on the
        // other end of the connection. The signal can be suppressed with
        // signal(SIGPIPE, SIG_IGN) but that simple solution must not be used
        // here; See other test 'libssl_connection_error_handling'.
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should not silently crash the program with signal "
                     "\"broken pipe\".\n";
        EXPECT_DEATH(sockObj.sock_ssl_connect(&info), ".*"); // Wrong!

    } else {

        // This expects NO segfault because it manages the SIGPIPE signal with a
        // special handler.
        EXPECT_EXIT((sockObj.sock_ssl_connect(&info), exit(0)),
                    ExitedWithCode(0), ".*");
    }

#else

    // This expects NO segfault because Winsock2 and BSD with setsockopt
    // SO_NOSIGPIPE does not generate a SIGPIPE signal.
    ASSERT_EXIT((sockObj.sock_ssl_connect(&info), exit(0)), ExitedWithCode(0),
                ".*");
    int ret_sock_ssl_connect = sockObj.sock_ssl_connect(&info);
    EXPECT_EQ(ret_sock_ssl_connect, UPNP_E_SOCKET_ERROR)
        << errStrEx(ret_sock_ssl_connect, UPNP_E_SOCKET_ERROR);
#endif

    EXPECT_EQ(CLOSE_SOCKET_P(info.socket), 0);
    SSL_CTX_free(gSslCtx);
}

#if 0
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
    return gtest_return_code; // managed in compa/gtest_main.inc
}
