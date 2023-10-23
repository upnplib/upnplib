// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-24

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS ::pupnp
#include <pupnp/sock.hpp>
#else
#define NS ::compa
#include <compa/sock.hpp>
#endif

#include <upnp.hpp>

#include <upnplib/port.hpp>
#include <upnplib/general.hpp>
#include "upnplib/upnptools.hpp" // for errStrEx

#include <openssl/err.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif

#include <utest/utest.hpp>

UPNPLIB_EXTERN SSL_CTX* gSslCtx;


namespace utest {

using ::testing::ExitedWithCode;

using upnplib::errStrEx;


// Helper Classes
// ==============
// I use these simple classes to ensure that we always free resources also in
// case of aborted tests without extra error checking. --Ingo

// Provide a socket
class CSockFd {
  public:
    SOCKET fd{};

    CSockFd() {
        TRACE("construct compa::CSockFd");
        this->fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->fd == -1) {
            throw std::runtime_error(std::string("Failed to get a socket."));
        }
    }
    virtual ~CSockFd() {
        TRACE("destruct compa::CSockFd");
        CLOSE_SOCKET_P(this->fd);
    }
};

// Provide the global SSL Context
class CGsslCtx {
  public:
    CGsslCtx() {
        TRACE("construct compa::CGsslCtx");
        gSslCtx = SSL_CTX_new(TLS_method());
        if (gSslCtx == nullptr) {
            throw std::runtime_error(std::string(
                "Failed to initialize the global SSL Context (gSslCtx)."));
        }
    }
    virtual ~CGsslCtx() {
        TRACE("destruct compa::CGsslCtx");
        SSL_CTX_free(gSslCtx);
    }
};


// OpenSSL TestSuite
//==================

class SockFTestSuite : public ::testing::Test {
#ifdef _WIN32
  protected:
    // Initialize Windows sochets
    SockFTestSuite() {
        WSADATA wsaData;
        int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc != NO_ERROR) {
            throw std::runtime_error(
                std::string("Failed to start Windows sockets (WSAStartup)."));
        }
    }

    // Cleanup Windows sochets
    ~SockFTestSuite() override { WSACleanup(); }
#endif
};
typedef SockFTestSuite SockFDeathTest;


TEST_F(SockFTestSuite, libssl_connection_error_handling) {
    // This test shows how to use libssl to initialize a connection and handle
    // some errors. It does not test a Unit. For discussion of the SIGPIPE
    // signal abort have a look at: https://stackoverflow.com/q/108183/5014688
    // and
    // http://www.microhowto.info/howto/ignore_sigpipe_without_affecting_other_threads_in_a_process.html.
    // Signal handlers are process global so we should not use signal() since
    // it's extremely bad behavior for a library to alter the caller's signal
    // handlers.

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

    // errno reports different error on different platforms
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
    EXPECT_EQ(stdErr.str(), "");

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
    CSockFd sock;
    info.socket = sock.fd;
    // c) initialize the global SSL Context in external variable gSslCtx;
    CGsslCtx gSslCtxObj;
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
        ASSERT_DEATH(sockObj.sock_ssl_connect(&info), ".*"); // Wrong!

    } else {

        // This expects NO program crash because it manages the SIGPIPE signal
        // with a special handler.
        ASSERT_EXIT((sockObj.sock_ssl_connect(&info), exit(0)),
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
}

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
