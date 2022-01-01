// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-01

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "genlib/net/sock.cpp"

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
// testsuite for the sock module
//==============================
TEST(EmptyTestSuite, empty_gtest) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
