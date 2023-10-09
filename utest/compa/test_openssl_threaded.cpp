// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-09

#include <upnplib/general.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/cmake_vars.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif

#include <gmock/gmock.h>

#include <cstring>
#include <thread>

namespace utest {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

// Simple TLS Server
// =================
// Inspired by https://wiki.openssl.org/index.php/Simple_TLS_Server and
// https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=examples-server-application-ssl-code

SOCKET create_socket(uint16_t port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
#ifdef _WIN32
        std::clog << "[Server:" << __LINE__
                  << "] Error - creating socket: WSAGetLastError()="
                  << WSAGetLastError() << ".\n";
#else
        std::clog << "[Server:" << __LINE__
                  << "] Error - creating socket: errno(" << errno << ")=\""
                  << std::strerror(errno) << "\".\n";
#endif
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
#ifdef _WIN32
        std::clog << "[Server:" << __LINE__
                  << "] Error - bind socket: WSAGetLastError()="
                  << WSAGetLastError() << ".\n";
#else
        std::clog << "[Server:" << __LINE__ << "] Error - bind socket: errno("
                  << errno << ")=\"" << std::strerror(errno) << "\".\n";
#endif
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) == -1) {
#ifdef _WIN32
        std::clog << "[Server:" << __LINE__
                  << "] Error - listen on socket: WSAGetLastError()="
                  << WSAGetLastError() << ".\n";
#else
        std::clog << "[Server:" << __LINE__
                  << "] Error - listen on socket: errno(" << errno << ")=\""
                  << std::strerror(errno) << "\".\n";
#endif
        exit(EXIT_FAILURE);
    }

    return s;
}

SSL_CTX* create_context() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(
            ctx, UPNPLIB_PROJECT_SOURCE_DIR "/gtests/cert.pem",
            SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(
            ctx, UPNPLIB_PROJECT_SOURCE_DIR "/gtests/key.pem",
            SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void simple_TLS_server() {
    TRACE("executing utest::simple_TLS_server()");

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
        exit(EXIT_FAILURE);
#else
    /* Ignore broken pipe signals */
    // signal(SIGPIPE, SIG_IGN);
#endif

    SOCKET sock = create_socket(4433);
    SSL_CTX* ctx = create_context();
    configure_context(ctx);

    /* Handle connections */
    while (1) {
        TRACE("  simple_TLS_server: loop next connection");
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL* ssl;
        const char reply[] = "test\n";

        SOCKET client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client == INVALID_SOCKET) {
            TRACE("  simple_TLS_server: EXIT - socket connection not accepted");
#ifdef _WIN32
            std::clog << "[Server:" << __LINE__
                      << "] Error - socket accept: WSAGetLastError()="
                      << WSAGetLastError() << ".\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
            exit(EXIT_FAILURE);
        }
        TRACE("  simple_TLS_server: socket connection accepted");

        ssl = SSL_new(ctx);
        // Due to man SSL_set_fd the type cast (int) is no problem.
        SSL_set_fd(ssl, (int)client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            TRACE("  simple_TLS_server: SSL connection with socket accepted, "
                  "send respond");
            // type cast (int) is no problem because there is a small buffer
            SSL_write(ssl, reply, (int)strlen(reply));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close((int)client);
    }

    close((int)sock);
    SSL_CTX_free(ctx);
}


// simple TLS client
//==================
// Inspired by https://stackoverflow.com/a/41321247/5014688. Thanks to O.logN

SSL* ssl_client;
SOCKET sock;

int SSL_error_print(const SSL* a_ssl, const int a_result) {
    const int ssl_error = SSL_get_error(a_ssl, a_result);
    const int err_no = errno;
    switch (ssl_error) {
    case SSL_ERROR_NONE:
        break;
    case SSL_ERROR_ZERO_RETURN:
        std::clog << "[Client:" << __LINE__ << "] SSL_ERROR_ZERO_RETURN\n";
        break;
    case SSL_ERROR_WANT_READ:
        std::clog << "[Client:" << __LINE__
                  << "] SSL_ERROR_WANT_READ, the operation did not complete "
                     "and can be retried later.\n";
        break;
    case SSL_ERROR_WANT_WRITE:
        std::clog << "[Client:" << __LINE__
                  << "] SSL_ERROR_WANT_WRITE, the operation did not complete "
                     "and can be retried later.\n";
        break;
    case SSL_ERROR_WANT_CONNECT:
        std::clog
            << "[Client:" << __LINE__
            << "] SSL_ERROR_WANT_CONNECT, the operation did not complete; the "
               "same TLS/SSL I/O function should be called again later.\n";
        break;
    case SSL_ERROR_WANT_ACCEPT:
        std::clog
            << "[Client:" << __LINE__
            << "] SSL_ERROR_WANT_ACCEPT, the operation did not complete; the "
               "same TLS/SSL I/O function should be called again later.\n";
        break;
    case SSL_ERROR_WANT_X509_LOOKUP:
        std::clog
            << "[Client:" << __LINE__
            << "] SSL_ERROR_WANT_X509_LOOKUP, the operation did not complete; "
               "the "
               "same TLS/SSL I/O function should be called again later.\n";
        break;
    case SSL_ERROR_WANT_ASYNC:
        std::clog
            << "[Client:" << __LINE__
            << "] SSL_ERROR_WANT_ASYNC, the operation did not complete; the "
               "same TLS/SSL I/O function should be called again later.\n";
        break;
    case SSL_ERROR_WANT_ASYNC_JOB:
        std::clog << "[Client:" << __LINE__ << "] SSL_ERROR_WANT_ASYNC_JOB.\n";
        break;
    case SSL_ERROR_SYSCALL:
        ERR_print_errors_fp(stdout);
        std::clog << "[Client:" << __LINE__
                  << "] SSL_ERROR_SYSCALL, some non-recoverable, fatal I/O "
                     "error occurred. errno("
                  << err_no << "): " << std::strerror(err_no) << "\n";
        break;
    case SSL_ERROR_SSL:
        std::clog << "[Client:" << __LINE__
                  << "] SSL_ERROR_SSL, a non-recoverable, fatal error in the "
                     "SSL library occurred, usually a protocol error.\n";
        break;
    default:
        std::clog << "[Client:" << __LINE__ << "] Unknown SSL_ERROR.\n";
        break;
    }
    errno = err_no;
    return ssl_error;
}

int RecvPacket() {
    int len = 100;
    char buf[1000000];
    std::cout << "[Client] received:\n";
    ERR_clear_error(); // must be empty to get correct SSL_get_error()
    do {
        len = SSL_read(ssl_client, buf, 100);
        buf[len] = 0;
        std::cout << std::string(buf);
    } while (len > 0);
    return SSL_error_print(ssl_client, len);
}

int SendPacket(const char* buf) {
    ERR_clear_error(); // must be empty to get correct SSL_get_error()
    int len = SSL_write(ssl_client, buf, (int)strlen(buf));
    return SSL_error_print(ssl_client, len);
}

int simple_TLS_client() {
    const SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        std::clog << "[Client:" << __LINE__
                  << "] Error - creating socket: errno(" << errno << ")=\""
                  << std::strerror(errno) << "\"\n";
        return -1;
    }

    constexpr addrinfo hints{{}, AF_INET, {}, {}, {}, {}, {}, {}};
    addrinfo* res{};
    constexpr char service[]{"4433"};
    int ret = getaddrinfo("localhost", service, &hints, &res);
    if (ret != 0) {
        std::clog << "[Client:" << __LINE__
                  << "] Error - getting address info: errid(" << ret << ")=\""
                  << gai_strerror(ret) << "\".\n";
        return -1;
    }

    constexpr int attempts{5};
    constexpr int delay{5}; // milliseconds
    int i;
    for (i = 0; i < attempts; i++) {
        if (connect(s, res->ai_addr, (socklen_t)sizeof(sockaddr)) == 0)
            break;
        std::clog << "[Client:" << __LINE__
                  << "] Error - connect socket: errno(" << errno << ")=\""
                  << std::strerror(errno) << "\". Try again after " << delay
                  << " ms...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
    if (i >= attempts) {
        std::clog << "[Client:" << __LINE__
                  << "] Error - give up connecting socket after " << i + 1
                  << " attempts.\n";
        return -1;
    }

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD* meth = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(meth);
    ssl_client = SSL_new(ctx);
    if (!ssl_client) {
        std::clog << "[Client:" << __LINE__ << "] Error - creating SSL.\n";
        ERR_print_errors_fp(stderr);
        return -1;
    }
    sock = SSL_get_fd(ssl_client);
    // Due to man SSL_set_fd the type cast (int) is no problem.
    SSL_set_fd(ssl_client, (int)s);
    int err = SSL_connect(ssl_client);
    if (err <= 0) {
        std::clog << "[Client:" << __LINE__
                  << "] Error - creating SSL connection, err=" << err << "\n";
        ERR_print_errors_fp(stderr);
        return -1;
    }
    printf("[Client] SSL connection using %s\n", SSL_get_cipher(ssl_client));

    char request[] = "GET https://about.google/intl/en/ HTTP/1.1\r\n\r\n";
    if (SendPacket(request) != SSL_ERROR_NONE) {
        std::clog << "[Client:" << __LINE__ << "] Error - send packet."
                  << std::endl;
        return -1;
    }
    if (RecvPacket() != SSL_ERROR_ZERO_RETURN) {
        std::clog << "[Client:" << __LINE__ << "] Error - receive packet."
                  << std::endl;
        return -1;
    }
    return 0;
}


// TEST(SockTestSuite, simple_tls_server) { EXPECT_EQ(simple_TLS_client(), 0); }

} // namespace utest

int main(int argc, char** argv) {
    std::thread t1(utest::simple_TLS_server);
    t1.detach();
    ::testing::InitGoogleMock(&argc, argv);
#include "gtest_main.inc"
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef _WIN32
    WSACleanup();
#endif
    TRACE("Program end.");
    return gtest_return_code; // managed in gtest_main.inc
}
