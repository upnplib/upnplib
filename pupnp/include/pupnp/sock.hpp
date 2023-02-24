#ifndef PUPNP_INCLUDE_PUPNP_SOCK_HPP
#define PUPNP_INCLUDE_PUPNP_SOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-25

#include "compa/interface_sock.hpp"

namespace compa {

// Interface for the sock module
// =============================
// clang-format off

class Csock : SockInterface {
  public:
    virtual ~Csock() override = default;

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
    int sock_close(SOCKET sock) override {
        return ::sock_close(sock); }
};
// clang-format on

} // namespace compa

#endif // PUPNP_INCLUDE_PUPNP_SOCK_HPP
