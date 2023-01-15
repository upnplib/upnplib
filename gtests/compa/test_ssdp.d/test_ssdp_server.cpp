// Copyright (C) 2023 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-15

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/ssdp/ssdp_server.cpp"
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS ::compa
#endif

#include "upnp.hpp" // for UPNP_E_* constants

#include "upnplib/port.hpp"

#include "gmock/gmock.h"

//
namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
// The ssdp_server call stack for some functions
//==============================================
/* clang-format off
   This is a simpliefied pseudo call stack for overview:

01)  get_ssdp_sockets()
     |
#ifdef INCLUDE_CLIENT_APIS
     |__ create_ssdp_sock_reqv4() // for SSDP REQUESTS
     |__ create_ssdp_sock_reqv6() // for SSDP REQUESTS
#endif
     |__ create_ssdp_sock_v4()         // for SSDP
     |__ create_ssdp_sock_v6()         // for SSDP
     |__ create_ssdp_sock_v6_ula_gua() // for SSDP

01) Creates the IPv4 and IPv6 ssdp sockets required by the control point and
    device operation.
clang-format on */

//
// ssdp_server TestSuite
// =====================
class SSDPserverFTestSuite : public ::testing::Test {
#ifdef _WIN32
    // Initialize and cleanup Windows sochets
  protected:
    SSDPserverFTestSuite() {
        WSADATA wsaData;
        int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc != NO_ERROR) {
            throw std::runtime_error(
                std::string("Failed to start Windows sockets (WSAStartup)."));
        }
    }

    ~SSDPserverFTestSuite() override { WSACleanup(); }
#endif
};

TEST_F(SSDPserverFTestSuite, create_ssdp_sock_reqv4) {
    SOCKET ssdpReqSock{};

    EXPECT_EQ(create_ssdp_sock_reqv4(&ssdpReqSock), UPNP_E_SUCCESS);
    EXPECT_GT(ssdpReqSock, (SOCKET)2);
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    // class CLogging loggingObj; // Output only with build type DEBUG.
#include "compa/gtest_main.inc"
}
