// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-25

#include "upnplib/socket.hpp"

#include "gmock/gmock.h"

using ::testing::ThrowsMessage;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

TEST(SockAddrTestSuite, successful_execution) {
    struct SockAddr sock;
    EXPECT_EQ(sock.addr_in->sin_family, AF_INET);

    EXPECT_EQ(sock.addr_get(), "0.0.0.0");
    EXPECT_EQ(sock.addr_get_port(), 0);

    sock.addr_set("192.168.169.170", 4711);
    EXPECT_EQ(sock.addr_get(), "192.168.169.170");
    EXPECT_EQ(sock.addr_get_port(), 4711);

    char buf_ntop[INET6_ADDRSTRLEN]{};
    EXPECT_NE(
        inet_ntop(AF_INET, &sock.addr_in->sin_addr, buf_ntop, sizeof(buf_ntop)),
        nullptr);
    EXPECT_STREQ(buf_ntop, "192.168.169.170");

    EXPECT_EQ(ntohs(sock.addr_in->sin_port), 4711);
}

TEST(SockAddrTestSuite, set_wrong_address) {
    struct SockAddr sock;

    // sock.addr_in->sin_addr.s_addr = 0xFFFFFFFF;

    EXPECT_THAT([&sock] { sock.addr_set("192.168.169.999", 4711); },
                ThrowsMessage<std::invalid_argument>(
                    "at */sock.cpp[22]: Invalid ip address '192.168.169.999'"));
}

TEST(SockAddrTestSuite, get_wrong_address) {
    struct SockAddr sock;

    // This is a wrong family entry that should trigger an error on getting an
    // ip address
    sock.addr_ss.ss_family = -1;

    EXPECT_THAT([&sock] { sock.addr_get(); },
                ThrowsMessage<std::runtime_error>(
                    "at */sock.cpp[35]: Got invalid ip address"));
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
}
