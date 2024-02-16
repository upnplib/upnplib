// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-18

// -----------------------------------------------------------------------------
// This testsuite starts the sample TV Device with general command line
// arguments and steps down the call stack with tests of the called functions so
// we can see what is needed to have the TV Device started. It is important to
// always stop the device after each test. Otherwise we get side effects on
// following tests.
// -----------------------------------------------------------------------------

#include <sample/common/tv_device.cpp>

#include <upnpapi.hpp>
#include <membuffer.hpp>
#include <ssdplib.hpp>

#include <pupnp/upnpdebug.hpp>

#include <upnplib/upnptools.hpp>
#include <upnplib/cmake_vars.hpp>
#include <upnplib/sockaddr.hpp>

#include <utest/utest_unix.hpp>
#include <umock/ifaddrs_mock.hpp>
#include <umock/net_if_mock.hpp>
#include <umock/sys_socket_mock.hpp>

#include <arpa/inet.h>

extern membuffer gDocumentRootDir;


// The tv_device call stack to use the pupnp library
//==================================================
/*
clang-format off

     main() // Starting in tv_device_main.cpp
     |__ device_main()
     |   |__ SampleUtil_Initialize()
     |   |__ 'Parse command line options'
     |   |__ TvDeviceStart()
     |       |__ pthread_mutex_init()
     |       |__ UpnpSetLogFileNames()
     |       |__ UpnpSetLogLevel()
     |       |__ UpnpInitLog()
     |       |
     |       |__ UpnpInit2()
     |       |__ if error
     |              UpnpFinish()
     |       |
     |       |__ switch ip_mode
     |       |     UpnpGetServerIpAddress()
     |       |     UpnpGetServerPort()
     |       |   or
     |       |     UpnpGetServerIp6Address()
     |       |     UpnpGetServerPort6()
     |       |   or
     |       |     UpnpGetServerUlaGuaIp6Address()
     |       |     UpnpGetServerUlaGuaPort6()
     |       |
24)  |       |__ UpnpSetWebServerRootDir()
     |       |__ if error
     |              UpnpFinish()
     |       |
28)  |       |__ UpnpRegisterRootDevice3()
     |       |__ if error
     |              UpnpFinish()
     |       |
     |       |__ TvDeviceStateTableInit()
     |       |__ UpnpSendAdvertisement()
     |       |__ if error
     |              UpnpFinish()
     |
     |__ pthread_create()
     |#ifdef _MSC_VER
     |__ pthread_join()
     |#else
     |__ 'Catch Ctrl-C for shutdown'
     |__ TvDeviceStop()

24) Tested within utest/compa/api.d/test_upnpapi.cpp
28) Tested within utest/compa/api.d/test_upnpapi.cpp

clang-format on
*/

namespace utest {

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::Ge;
using ::testing::InSequence;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;

using ::pupnp::CLogging;

using ::utest::CIfaddr4;

using ::upnplib::errStrEx;
using ::upnplib::SSockaddr;


class SampleTvDeviceFTestSuite : public ::testing::Test {
  protected:
    // constructor of this testsuite
    SampleTvDeviceFTestSuite() {
        // initialize needed global variables
        std::fill(std::begin(gIF_NAME), std::end(gIF_NAME), 0);
        std::fill(std::begin(gIF_IPV4), std::end(gIF_IPV4), 0);
        std::fill(std::begin(gIF_IPV4_NETMASK), std::end(gIF_IPV4_NETMASK), 0);
        std::fill(std::begin(gIF_IPV6), std::end(gIF_IPV6), 0);
        gIF_IPV6_PREFIX_LENGTH = 0;
        std::fill(std::begin(gIF_IPV6_ULA_GUA), std::end(gIF_IPV6_ULA_GUA), 0);
        gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
        gIF_INDEX = -1u;
    }
};


#if false
TEST_F(SampleTvDeviceFTestSuite, valid_commandline_arguments) {
    // IMPORTANT! There is a limit FD_SETSIZE = 1024 for socket file
    // descriptors that can be used with 'select()'. This limit is not given
    // when using 'poll()' or 'epoll' instead. Old_code uses 'select()' so we
    // must test with this limited socket file descriptors. Otherwise we may
    // get segfaults with 'FD_SET()'. For details have a look at 'man select'.

    // SKIP on Github Actions
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Provide commandline values
    constexpr int argc{3};
    constexpr int argsize = sizeof(UPNPLIB_PROJECT_BINARY_DIR "/bin/tv_device");
    char arg[argc][argsize]{UPNPLIB_PROJECT_BINARY_DIR "/bin/tv_device",
                            "--webdir", SAMPLE_SOURCE_DIR "/web"};
    char* argv[argc]{arg[0], arg[1], arg[2]};

    // Define used network parameter
    const std::string ip_addr_str{"192.168.99.4"};

    // Provide "local" network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", ip_addr_str + "/24");
    ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    // Mock system functions
    umock::IfaddrsMock ifaddrsObj;
    umock::Ifaddrs ifaddrs_injectObj(&ifaddrsObj);
    EXPECT_CALL(ifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(ifaddrsObj, freeifaddrs(ifaddr)).Times(1);

    umock::Net_ifMock net_ifObj;
    umock::Net_if net_if_injectObj(&net_ifObj);
    EXPECT_CALL(net_ifObj, if_nametoindex(_)).WillOnce(Return(2));

    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    ON_CALL(sys_socketObj, connect(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));

    { // begin scope InSequence
        InSequence seq;

        // *** Start mock UpnpInit2() on tv_device.cpp -> TvDeviceStart ***
        // ****************************************************************
        // Mock V4 and V6 http listeners: get socket, bind it, listen on it and
        // get port with getsockname().
        constexpr SOCKET listen_sockfd{umock::sfd_base + 82};
        const std::string listen_port{"50008"};
        SSockaddr
            listen_ssObj; // for getsockname() return sockaddr & port
        listen_ssObj = ip_addr_str + ":" + listen_port;

        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
            .WillOnce(Return(listen_sockfd));
        EXPECT_CALL(sys_socketObj, bind(listen_sockfd, _, _)).Times(1);
        EXPECT_CALL(sys_socketObj, listen(listen_sockfd, SOMAXCONN)).Times(1);
        EXPECT_CALL(
            sys_socketObj,
            getsockname(listen_sockfd, _, Pointee(Ge(sizeof(sockaddr_in)))))
            .WillOnce(DoAll(SetArgPointee<1>(*(sockaddr*)&listen_ssObj.ss),
                            Return(0)));

        // Mock stop socket (to end miniserver processing): get socket, bind it
        // and get port with getsockname().
        constexpr SOCKET stop_sockfd{umock::sfd_base + 83};
        const std::string stop_port{"50009"};
        SSockaddr
            stop_ssObj; // for getsockname() return sockaddr & port
        stop_ssObj = ip_addr_str + ":" + stop_port;

        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        EXPECT_CALL(sys_socketObj, bind(stop_sockfd, _, _)).Times(1);
        EXPECT_CALL(
            sys_socketObj,
            getsockname(stop_sockfd, _, Pointee(Ge(sizeof(sockaddr_in)))))
            .WillOnce(
                DoAll(SetArgPointee<1>(*(sockaddr*)&stop_ssObj.ss), Return(0)));

        // Mock SSDP socket for discovery/advertising: get socket, bind it.
        // Create the IPv4 socket for SSDP REQUESTS
        constexpr SOCKET ssdpreq_sockfd{umock::sfd_base + 84};
        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(ssdpreq_sockfd));
        EXPECT_CALL(sys_socketObj, setsockopt(ssdpreq_sockfd, IPPROTO_IP,
                                              IP_MULTICAST_TTL, _, _));
        // Create the IPv4 socket for SSDP
        constexpr SOCKET ssdp_sockfd{umock::sfd_base + 85};
        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(ssdp_sockfd));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdp_sockfd, SOL_SOCKET, SO_REUSEADDR, _, _));
        EXPECT_CALL(sys_socketObj, bind(ssdp_sockfd, _, _)).Times(1);
        EXPECT_CALL(sys_socketObj, setsockopt(ssdp_sockfd, IPPROTO_IP,
                                              IP_ADD_MEMBERSHIP, _, _));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdp_sockfd, IPPROTO_IP, IP_MULTICAST_IF, _, _));
        EXPECT_CALL(sys_socketObj, setsockopt(ssdp_sockfd, IPPROTO_IP,
                                              IP_MULTICAST_TTL, _, _));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdp_sockfd, SOL_SOCKET, SO_BROADCAST, _, _));
        // .WillOnce(Return(-1)); // This will help to find the program line
        // **************************************************************
        // *** End mock UpnpInit2() on tv_device.cpp -> TvDeviceStart ***

        // if (!old_code) {
        //     EXPECT_CALL(sys_socketObj,
        //                 getsockopt(listen_sockfd, SOL_SOCKET, SO_ERROR, _,
        //                 _));
        // }
    } // end scope InSequence

    // Test Unit
    int ret_device_main = device_main(argc, argv);
    EXPECT_EQ(ret_device_main, UPNP_E_SUCCESS)
        << errStrEx(ret_device_main, UPNP_E_SUCCESS);

    // Stop device
    // EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}
#endif

TEST_F(SampleTvDeviceFTestSuite, invalid_commandline_argument) {
    constexpr int argc{5};
    char arg[argc][20]{"build/bin/tv_device", "-i", "ens2", "--webdir",
                       "./sample/web"};
    char* argv[argc]{arg[0], arg[1], arg[2], arg[3], arg[4]};

    // argc is 0 so the function should return immediately with error
    EXPECT_EQ(device_main(0, argv), UPNP_E_INVALID_ARGUMENT);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

TEST_F(SampleTvDeviceFTestSuite, TvDeviceStart_successful) {
    CLogging logObj; // Output only with build type DEBUG.
    logObj.enable(UPNP_ALL);

    // Arguments of TvDeviceStart()
    constexpr char* iface{};
    constexpr in_port_t port{};
    constexpr char* desc_doc_name{};
    // constexpr char web_dir_path[]{SAMPLE_SOURCE_DIR "/web\0"};
    constexpr int ip_mode = IP_MODE_IPV4;

    // Mock system functions
    umock::IfaddrsMock ifaddrsObj;
    umock::Ifaddrs ifaddrs_injectObj(&ifaddrsObj);
    umock::Net_ifMock net_ifObj;
    umock::Net_if net_if_injectObj(&net_ifObj);
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);

    // Provide values for a local network interface
    const std::string local_addr_str{"192.168.99.4"};

    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", local_addr_str + "/24");
    ifaddrs* ifaddr = ifaddr4Obj.get();

    EXPECT_CALL(ifaddrsObj, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(ifaddrsObj, freeifaddrs(ifaddr)).Times(1);
    // Interface index set to 2
    EXPECT_CALL(net_ifObj, if_nametoindex(_)).WillOnce(Return(2));

    // Set default socket object values
    ON_CALL(sys_socketObj, socket(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EACCES, SOCKET_ERROR));
    ON_CALL(sys_socketObj, select(_, _, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
    ON_CALL(sys_socketObj, connect(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
    ON_CALL(sys_socketObj, bind(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
    ON_CALL(sys_socketObj, listen(_, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
    ON_CALL(sys_socketObj, setsockopt(_, _, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));

    { // begin scope InSequence
        InSequence seq;

        // *** Start mock UpnpInit2() on tv_device.cpp -> TvDeviceStart ***
        // ****************************************************************
        // Mock V4 and V6 http listeners: get socket, bind it, listen on it and
        // get port with getsockname().
        constexpr SOCKET listen_sockfd{umock::sfd_base + 41};
        const std::string listen_port_str{"50010"};
        SSockaddr listen_ssObj; // for getsockname() return sockaddr & port
        listen_ssObj = local_addr_str + ":" + listen_port_str;

        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
            .WillOnce(Return(listen_sockfd));
        EXPECT_CALL(sys_socketObj, bind(listen_sockfd, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(sys_socketObj, listen(listen_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        EXPECT_CALL(
            sys_socketObj,
            getsockname(listen_sockfd, _, Pointee(Ge(sizeof(sockaddr_in)))))
            .WillOnce(DoAll(SetArgPointee<1>(*(sockaddr*)&listen_ssObj.ss),
                            Return(0)));

        // Mock stop socket (to end miniserver processing): get socket, bind it
        // and get port with getsockname().
        constexpr SOCKET stop_sockfd{umock::sfd_base + 42};
        constexpr SOCKET ssdpReqSock{umock::sfd_base + 43};
        constexpr SOCKET ssdpSock{umock::sfd_base + 44};
        const std::string stop_port{"50011"};
        SSockaddr stop_ssObj; // for getsockname() return sockaddr & port
        stop_ssObj = local_addr_str + ":" + stop_port;

        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        EXPECT_CALL(sys_socketObj, bind(stop_sockfd, _, _)).WillOnce(Return(0));
        EXPECT_CALL(
            sys_socketObj,
            getsockname(stop_sockfd, _, Pointee(Ge(sizeof(sockaddr_in)))))
            .WillOnce(
                DoAll(SetArgPointee<1>(*(sockaddr*)&stop_ssObj.ss), Return(0)));
        // Mock ssdp_server: create_ssdp_sock_reqv4()
        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(ssdpReqSock));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpReqSock, IPPROTO_IP, IP_MULTICAST_TTL, _, _))
            .WillOnce(Return(0));
        // Mock ssdp_server: create_ssdp_sock_v4()
        EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(ssdpSock));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, SOL_SOCKET, SO_REUSEADDR, _, _))
            .WillOnce(Return(0));
#if (defined(BSD) && !defined(__GNU__)) || defined(__APPLE__)
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, SOL_SOCKET, SO_REUSEPORT, _, _))
            .WillOnce(Return(0));
#endif
        EXPECT_CALL(sys_socketObj,
                    bind(ssdpSock, Pointee(A<sockaddr>()), sizeof(sockaddr_in)))
            .WillOnce(Return(0));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, IPPROTO_IP, IP_MULTICAST_IF, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, IPPROTO_IP, IP_MULTICAST_TTL, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(sys_socketObj,
                    setsockopt(ssdpSock, SOL_SOCKET, SO_BROADCAST, _, _))
            .WillOnce(Return(0));
    } // end scope InSequence


    // Test Unit
    // with default settings as far as possible. With nullptr for the
    // webdir path, the hard coded webdir path is used.
    int ret_TvDeviceStart =
        TvDeviceStart(iface, port, desc_doc_name, nullptr /*web_dir_path*/,
                      ip_mode, linux_print, 0);
    EXPECT_EQ(ret_TvDeviceStart, UPNP_E_SUCCESS)
        << errStrEx(ret_TvDeviceStart, UPNP_E_SUCCESS);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

} // namespace utest


int main(int argc, char** argv) {
    if (std::getenv("GITHUB_ACTIONS"))
        return 0;
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
