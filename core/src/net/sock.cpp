// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-21

#include "upnplib/sock.hpp"
#include "mocking/sys_socket.hpp"
#include <filesystem>
#include <cstring>

namespace upnplib {

// Wrapper for a sockaddr structure
// --------------------------------
SockAddr::SockAddr() {
    this->addr_ss.ss_family = AF_INET;
    this->addr = (struct sockaddr*)&this->addr_ss;
}

void SockAddr::addr_set(const std::string& a_text_addr, unsigned short a_port) {
    int ret = inet_pton(this->addr_ss.ss_family, a_text_addr.c_str(),
                        &this->addr_in->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            a_text_addr + "'");
    }
    this->addr_in->sin_port = htons(a_port);
}

std::string SockAddr::addr_get() {
    // Return the text address which is stored in this structure.

    char buf_ntop[INET6_ADDRSTRLEN]{};

    const char* text_addr =
        inet_ntop(this->addr_ss.ss_family, &this->addr_in->sin_addr, buf_ntop,
                  sizeof(buf_ntop));
    if (text_addr == nullptr)
        throw std::invalid_argument(
            "UPnPlib ERR. at */" +                                //
            std::filesystem::path(__FILE__).filename().string() + //
            "[" + std::to_string(__LINE__) + "], " + __FUNCTION__ +
            "(), errid=" + std::to_string(errno) + ": " + std::strerror(errno));

    return std::string(buf_ntop);
}

unsigned short SockAddr::addr_get_port() {
    return ntohs(this->addr_in->sin_port);
}

std::string SocketAddr::addr_get() { return SockAddr::addr_get(); }

std::string SocketAddr::addr_get(SOCKET a_sockfd) {
    int rc = mocking::sys_socket_h.getsockname(
        a_sockfd, (struct sockaddr*)this->addr, &this->addr_len);

    if (rc == -1)
        throw std::runtime_error(
            "UPnPlib ERR. at */" +
            std::filesystem::path(__FILE__).filename().string() + "[" +
            std::to_string(__LINE__) + "], " + __FUNCTION__ + "(" +
            std::to_string(a_sockfd) + "), errid=" + std::to_string(errno) +
            ": systemcall getsockname(" + std::to_string(a_sockfd) + "), " +
            std::strerror(errno));

    return SockAddr::addr_get();
}

} // namespace upnplib
