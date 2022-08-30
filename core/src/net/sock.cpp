// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-30

#include "upnplib/sock.hpp"
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
    if (text_addr == nullptr) {
        throw std::runtime_error(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Got invalid ip address");
    }

    return std::string(buf_ntop);
}

SocketAddr::SocketAddr(ISysSocket* a_sys_socketObj)
    : m_sys_socketObj(a_sys_socketObj) {}

std::string SocketAddr::addr_get() { return SockAddr::addr_get(); }

std::string SocketAddr::addr_get(SOCKET a_sockfd) {
    int rc = m_sys_socketObj->getsockname(
        a_sockfd, (struct sockaddr*)this->addr, &this->addr_len);

    if (rc == -1)
        throw std::runtime_error(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) +
            "]: Got invalid ip address from fd " + std::to_string(a_sockfd) +
            ". " + std::strerror(errno));

    return SockAddr::addr_get();
}

unsigned short SockAddr::addr_get_port() {
    return ntohs(this->addr_in->sin_port);
}

} // namespace upnplib
