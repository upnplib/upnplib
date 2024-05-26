// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-28

// All functions of the miniserver module have been covered by a gtest. Some
// tests are skipped and must be completed when missed information is
// available.

#include <miniserver.hpp>
#include <umock/pupnp_miniserver_mock.hpp>
#include <umock/pupnp_ssdp_mock.hpp>

// Include source code for testing. So we have also direct access to file local
// scoped functions which need to be tested.
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
// #include <Pupnp/upnp/src/api/upnpapi.cpp> // only for StartMiniServer_real
#include <Pupnp/upnp/src/genlib/miniserver/miniserver.cpp>
#else
// #include <Compa/src/api/upnpapi.cpp> // only for StartMiniServer_real
#include <Compa/src/genlib/miniserver/miniserver.cpp>
#endif

#include <upnplib/global.hpp>
#include <upnplib/upnptools.hpp> // for errStrEx
#include <upnplib/addrinfo.hpp>
#include <upnplib/socket.hpp>

#include <pupnp/upnpdebug.hpp>
#include <pupnp/threadpool_init.hpp>

#include <utest/utest.hpp>
#include <umock/sys_socket_mock.hpp>
#ifdef _MSC_VER
#include <umock/winsock2_mock.hpp>
#endif


namespace utest {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Ge;
using ::testing::InSequence;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;
using ::testing::StrictMock;

using ::upnplib::CAddrinfo;
using ::upnplib::CSocket_basic;
using ::upnplib::errStrEx;
using ::upnplib::g_dbug;
using ::upnplib::SSockaddr;

using ::pupnp::CLogging;
using ::pupnp::CThreadPoolInit;


// The miniserver call stack to get a server socket
//=================================================
// clang-format off
/* This test suite follows the successful calls to get a server socket:
   StartMiniServer()
   |
   |__ InitMiniServerSockArray()                   ]
   |__ get_miniserver_sockets()                    ]
   |   |__ init_socket_suff()                      ] create sockets
   |   |   |__ get a socket()                      ]
   |   |   |__ setsockopt() - MINISERVER_REUSEADDR |
   |   |                                           V
   |   |__ do_bind_listen()
   |       |
   |       |__ do_bind()              ] bind socket to ip address
   |       |   |__ bind()             ]
   |       |
   |       |__ do_listen()            ]
   |       |   |__ listen()           ] listen on a port,
   |       |                          ] and wait for a connection
   |       |__ if EADDRINUSE          ]
   |       |      init_socket_suff()  ]
   |       |
   |       |__ get_port()             ] get the current port
   |           |__ getsockname()      ]
   |                                               A
   |__ get_miniserver_stopsock()                   | create sockets
   |__ get_ssdp_sockets()                          ]
   |
   |__ TPJobInit() to RunMiniServer()              ]
   |__ TPJobSetPriority()                          ] Add MiniServer
   |__ TPJobSetFreeFunction()                      ] to ThreadPool
   |__ ThreadPoolAddPersistent()                   ]
   |__ while ("wait for miniserver to start")

StartMiniServer() has started RunMiniServer() as thread. It then waits blocked
with select() undefinetly (timeout set to NULL) until it receive from any
selected socket. StartMiniServer() also has started a miniserver stopsock() on
another thread. This thread polls receive_from_stopSock() for an incomming
SOCK_DGRAM message "ShutDown" send to stopsock bound to "127.0.0.1". This will
be recieved by select() so it will always enable a blocking (waiting) select().

   RunMiniServer() as thread, started by StartMiniServer()
   |__ while(receive_from_stopSock() not "ShutDown")
   |   |__ select()
   |   |__ web_server_accept()
   |   |   |__ accept()
   |   |   |__ schedule_request_job()
   |   |       |__ TPJobInit() to handle_request()
   |   |
   |   |__ ssdp_read()
   |   |__ block until receive_from_stopSock()
   |
   |__ sock_close()
   |__ free()

   handle_request() as thread, started by schedule_request_job()
   |__ http_RecvMessage()
*/
// clang-format on

// Miniserver TestSuite
// ====================
ACTION_P(set_gMServState, value) { gMServState = value; }

ACTION_P(debugCoutMsg, msg) {
    if (g_dbug)
        std::cout << msg << std::flush;
}

class StartMiniServerFTestSuite : public ::testing::Test {
  protected:
    CLogging logObj; // Output only with build type DEBUG.

    // Constructor
    StartMiniServerFTestSuite() {
        if (g_dbug)
            logObj.enable(UPNP_ALL);

        // Clean up needed global environment
        gIF_IPV4[0] = '\0';
        gIF_IPV6[0] = '\0';
        gIF_IPV6_ULA_GUA[0] = '\0';
        LOCAL_PORT_V4 = 0;
        LOCAL_PORT_V6 = 0;
        LOCAL_PORT_V6_ULA_GUA = 0;
        gMServState = MSERV_IDLE;
    }
};

class StartMiniServerMockFTestSuite : public StartMiniServerFTestSuite {
  protected:
    // Instantiate mocking objects.
    StrictMock<umock::Sys_socketMock> m_sys_socketObj;
    // Inject the mocking objects into the tested code.
    umock::Sys_socket sys_socket_injectObj{&m_sys_socketObj};

    StrictMock<umock::PupnpMiniServerMock> miniserverObj;
    umock::PupnpMiniServer miniserver_injectObj{&miniserverObj};

    StrictMock<umock::PupnpSsdpMock> ssdpObj;
    umock::PupnpSsdp ssdp_injectObj{&ssdpObj};
#ifdef _MSC_VER
    StrictMock<umock::Winsock2Mock> m_winsock2Obj;
    umock::Winsock2 winsock2_injectObj{&m_winsock2Obj};
#endif

    // Constructor
    StartMiniServerMockFTestSuite() {
        // Set default socket object values
        ON_CALL(m_sys_socketObj, socket(_, _, _))
            .WillByDefault(SetErrnoAndReturn(EACCES, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, bind(_, _, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, listen(_, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, select(_, _, _, _, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, getsockopt(_, _, _, _, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, setsockopt(_, _, _, _, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(m_sys_socketObj, getsockname(_, _, _))
            .WillByDefault(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
        ON_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillByDefault(Return(UPNP_E_OUTOF_SOCKET));
        ON_CALL(miniserverObj, RunMiniServer(_))
            .WillByDefault(set_gMServState(MSERV_IDLE));
#ifdef _MSC_VER
        ON_CALL(m_winsock2Obj, WSAGetLastError())
            .WillByDefault(Return(WSAENOTSOCK));
#endif
    }
};


// This test uses real connections and isn't portable. It is only for humans to
// see how it works and should not always enabled.
#if 0
TEST(StartMiniServerTestSuite, StartMiniServer_real) {
    // MINISERVER_REUSEADDR = false;
    // gIF_IPV4 = "";
    // LOCAL_PORT_V4 = 0;
    // LOCAL_PORT_V6 = 0;
    // LOCAL_PORT_V6_ULA_GUA = 0;

    ASSERT_FALSE(MINISERVER_REUSEADDR);

    // Perform initialization preamble.
    int ret_UpnpInitPreamble = UpnpInitPreamble();
    ASSERT_EQ(ret_UpnpInitPreamble, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitPreamble, UPNP_E_SUCCESS);

    // Retrieve interface information (Addresses, index, etc).
    int ret_UpnpGetIfInfo = UpnpGetIfInfo(nullptr);
    ASSERT_EQ(ret_UpnpGetIfInfo, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_SUCCESS);

    // Due to initialization by components it should not have flagged to be
    // initialized. That will we do now.
    ASSERT_EQ(UpnpSdkInit, 0);
    ASSERT_STRNE(gIF_IPV4, "");
    UpnpSdkInit = 1;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 1);
    EXPECT_EQ(LOCAL_PORT_V4, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(LOCAL_PORT_V6, 0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, 0);

    EXPECT_EQ(StopMiniServer(), 0); // Always returns 0

    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);
}
#endif


TEST_F(StartMiniServerFTestSuite, start_miniserver_with_no_ip_addr) {
    // Not having a valid local ip_address on all interfaces will fail to start
    // the miniserver. The global ip address variables gIF_IPV4, gIF_IPV6,
    // gIF_IPV6_ULA_GUA are used and cleared by the fixture.

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET);
}

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
TEST_F(StartMiniServerMockFTestSuite, start_miniserver_with_one_ipv4_addr) {
    std::strcpy(gIF_IPV4, "192.168.47.11");
    LOCAL_PORT_V4 = 50071;
    constexpr SOCKET listen_sockfd{umock::sfd_base + 86};
    constexpr SOCKET stop_sockfd{umock::sfd_base + 87};

    SSockaddr listen_ssObj; // for getsockname() return sockaddr & port
    listen_ssObj = std::string(gIF_IPV4) + ":" + std::to_string(LOCAL_PORT_V4);
    SSockaddr stop_ssObj;
    stop_ssObj = "127.0.0.1:0";

    { // begin scope InSequence
        InSequence seq;

        // Provide a socket id for listening.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
            .WillOnce(Return(listen_sockfd));
        // Bind socket to an ip address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listen_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on a socket
        EXPECT_CALL(m_sys_socketObj, listen(listen_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listen_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(SetArgPointee<1>(
                                *reinterpret_cast<sockaddr*>(&listen_ssObj.ss)),
                            Return(0)));

        // Provide a socket id for the stop socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        // Bind socket to an ip address for the stop socket
        EXPECT_CALL(m_sys_socketObj, bind(stop_sockfd, _, _))
            .WillOnce(Return(0));
        // getsockname() for stop socket
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(stop_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&stop_ssObj.ss)),
                Return(0)));

        // Mock get_ssdp_sockets
        EXPECT_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [int get_ssdp_sockets()] "
                                   "MOCK MSG1088: Executing with success...\n"),
                      Return(UPNP_E_SUCCESS)));

        // Mock RunMiniServer
        EXPECT_CALL(miniserverObj, RunMiniServer(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [void RunMiniServer()] "
                                   "MOCK MSG1090: Executing with success...\n"),
                      set_gMServState(MSERV_RUNNING)));
    } // end scope InSequence

    // We need the threadpool to RunMiniServer().
    CThreadPoolInit tp(gMiniServerThreadPool);
    // TPAttrSetMaxThreads(&gMiniServerThreadPool.attr, 0);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);
}

TEST_F(StartMiniServerMockFTestSuite, start_miniserver_with_one_ipv6_lla_addr) {
    std::strcpy(gIF_IPV6, "fe80::fedc:0:0:1");
    LOCAL_PORT_V6 = 50072;
    constexpr SOCKET listen_sockfd{umock::sfd_base + 88};
    [[maybe_unused]] constexpr SOCKET stop_sockfd{umock::sfd_base + 89};

    SSockaddr listen_ssObj; // for getsockname() return sockaddr & port
    listen_ssObj =
        "[" + std::string(gIF_IPV6) + "]:" + std::to_string(LOCAL_PORT_V6);
    SSockaddr stop_ssObj;
    stop_ssObj = "127.0.0.1:0";

    { // begin scope InSequence
        InSequence seq;

        // Provide a socket id for listening.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET6, SOCK_STREAM, 0))
            .WillOnce(Return(listen_sockfd));
        // Mock setsockopt()
        EXPECT_CALL(m_sys_socketObj, setsockopt(listen_sockfd, _, _, _, _))
            .WillOnce(Return(0));

#ifndef UPNPLIB_WITH_NATIVE_PUPNP
        // Bind socket to an ip address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listen_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on a socket
        EXPECT_CALL(m_sys_socketObj, listen(listen_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listen_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in6))))))
            .WillOnce(DoAll(SetArgPointee<1>(
                                *reinterpret_cast<sockaddr*>(&listen_ssObj.ss)),
                            Return(0)));

        // Provide a socket id for the stop socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        // Bind socket to an ip address for the stop socket
        EXPECT_CALL(m_sys_socketObj, bind(stop_sockfd, _, _))
            .WillOnce(Return(0));
        // getsockname() for stop socket
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(stop_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&stop_ssObj.ss)),
                Return(0)));

        // Mock get_ssdp_sockets
        EXPECT_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [int get_ssdp_sockets()] "
                                   "MOCK MSG1089: Executing with success...\n"),
                      Return(UPNP_E_SUCCESS)));

        // Mock RunMiniServer
        EXPECT_CALL(miniserverObj, RunMiniServer(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [void RunMiniServer()] "
                                   "MOCK MSG1090: Executing with success...\n"),
                      set_gMServState(MSERV_RUNNING)));
#endif
    } // end scope InSequence

    // We need the threadpool to RunMiniServer().
    CThreadPoolInit tp(gMiniServerThreadPool);
    // TPAttrSetMaxThreads(&gMiniServerThreadPool.attr, 0);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Having only one IPv6 lla address should not fail "
                     "StartMiniServer().\n";
        EXPECT_EQ(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET); // Wrong!

    } else {

        EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
            << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);
    }
}

TEST_F(StartMiniServerMockFTestSuite,
       start_miniserver_with_one_ipv6_ula_gua_addr) {
    std::strcpy(gIF_IPV6_ULA_GUA, "2001:db8::1");
    LOCAL_PORT_V6_ULA_GUA = 50073;
    constexpr SOCKET listen_sockfd{umock::sfd_base + 66};
    [[maybe_unused]] constexpr SOCKET stop_sockfd{umock::sfd_base + 67};

    SSockaddr listen_ssObj; // for getsockname() return sockaddr & port
    listen_ssObj = "[" + std::string(gIF_IPV6_ULA_GUA) +
                   "]:" + std::to_string(LOCAL_PORT_V6_ULA_GUA);
    SSockaddr stop_ssObj;
    stop_ssObj = "127.0.0.1:0";

    { // begin scope InSequence
        InSequence seq;

        // Provide a socket id for listening.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET6, SOCK_STREAM, 0))
            .WillOnce(Return(listen_sockfd));
        // Mock setsockopt()
        EXPECT_CALL(m_sys_socketObj, setsockopt(listen_sockfd, _, _, _, _))
            .WillOnce(Return(0));

#ifndef UPNPLIB_WITH_NATIVE_PUPNP
        // Bind socket to an ip address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listen_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on a socket
        EXPECT_CALL(m_sys_socketObj, listen(listen_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listen_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in6))))))
            .WillOnce(DoAll(SetArgPointee<1>(
                                *reinterpret_cast<sockaddr*>(&listen_ssObj.ss)),
                            Return(0)));

        // Provide a socket id for the stop socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        // Bind socket to an ip address for the stop socket
        EXPECT_CALL(m_sys_socketObj, bind(stop_sockfd, _, _))
            .WillOnce(Return(0));
        // getsockname() for stop socket
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(stop_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&stop_ssObj.ss)),
                Return(0)));

        // Mock get_ssdp_sockets
        EXPECT_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [int get_ssdp_sockets()] "
                                   "MOCK MSG1092: Executing with success...\n"),
                      Return(UPNP_E_SUCCESS)));

        // Mock RunMiniServer
        EXPECT_CALL(miniserverObj, RunMiniServer(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [void RunMiniServer()] "
                                   "MOCK MSG1093: Executing with success...\n"),
                      set_gMServState(MSERV_RUNNING)));
#endif
    } // end scope InSequence

    // We need the threadpool to RunMiniServer().
    CThreadPoolInit tp(gMiniServerThreadPool);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Having only one IPv6 ula/gua address should not fail "
                     "StartMiniServer().\n";
        EXPECT_EQ(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_StartMiniServer, UPNP_E_OUTOF_SOCKET); // Wrong!

    } else {

        EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
            << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);
    }
}

TEST_F(StartMiniServerMockFTestSuite,
       start_miniserver_with_ipv4_and_ipv6_addr) {
    std::strcpy(gIF_IPV4, "192.168.47.47");
    std::strcpy(gIF_IPV6_ULA_GUA, "2001:db8::2");
    LOCAL_PORT_V4 = 50077;
    LOCAL_PORT_V6_ULA_GUA = 50078;
    constexpr SOCKET listen4_sockfd{umock::sfd_base + 91};
    constexpr SOCKET listen6_sockfd{umock::sfd_base + 92};
    constexpr SOCKET stop_sockfd{umock::sfd_base + 93};

    SSockaddr listen4_ssObj; // for getsockname() IPv4 return sockaddr & port
    listen4_ssObj = std::string(gIF_IPV4) + ":" + std::to_string(LOCAL_PORT_V4);

    SSockaddr listen6_ssObj; // for getsockname() IPv6 return sockaddr & port
    listen6_ssObj = "[" + std::string(gIF_IPV6_ULA_GUA) +
                    "]:" + std::to_string(LOCAL_PORT_V6_ULA_GUA);

    SSockaddr stop_ssObj;
    stop_ssObj = "127.0.0.1:0";

    { // begin scope InSequence
        InSequence seq;

        // Provide a socket id for listening IPv4.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
            .WillOnce(Return(listen4_sockfd));
        // Provide a socket id for listening IPv6.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET6, SOCK_STREAM, 0))
            .WillOnce(Return(listen6_sockfd));
        // Mock setsockopt() for IPv6
        EXPECT_CALL(m_sys_socketObj, setsockopt(listen6_sockfd, _, _, _, _))
            .WillOnce(Return(0));
        // Bind socket to an IPv4 address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listen4_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on a IPv4 socket
        EXPECT_CALL(m_sys_socketObj, listen(listen4_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() from IPv4 for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listen4_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(
                                &listen4_ssObj.ss)),
                            Return(0)));
        // Bind socket to an IPv6 address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listen6_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on a IPv6 socket
        EXPECT_CALL(m_sys_socketObj, listen(listen6_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() from IPv6 for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listen6_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in6))))))
            .WillOnce(DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(
                                &listen6_ssObj.ss)),
                            Return(0)));

        // Provide a socket id for the stop socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        // Bind socket to an ip address for the stop socket
        EXPECT_CALL(m_sys_socketObj, bind(stop_sockfd, _, _))
            .WillOnce(Return(0));
        // getsockname() for stop socket
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(stop_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&stop_ssObj.ss)),
                Return(0)));

        // Mock get_ssdp_sockets
        EXPECT_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [int get_ssdp_sockets()] "
                                   "MOCK MSG1091: Executing with success...\n"),
                      Return(UPNP_E_SUCCESS)));

        // Mock RunMiniServer
        EXPECT_CALL(miniserverObj, RunMiniServer(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [void RunMiniServer()] "
                                   "MOCK MSG1090: Executing with success...\n"),
                      set_gMServState(MSERV_RUNNING)));
    } // end scope InSequence

    // We need the threadpool to RunMiniServer().
    CThreadPoolInit tp(gMiniServerThreadPool);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);
}

TEST_F(StartMiniServerMockFTestSuite,
       start_miniserver_with_ipv6_lla_and_ipv6_ula_gua_addr) {
    std::strcpy(gIF_IPV6, "fe80::fedc:0:0:2");
    std::strcpy(gIF_IPV6_ULA_GUA, "2001:db8::6");
    LOCAL_PORT_V6 = 50074;
    LOCAL_PORT_V6_ULA_GUA = 50075;
    constexpr SOCKET listenl_sockfd{umock::sfd_base + 68};
    constexpr SOCKET listeng_sockfd{umock::sfd_base + 69};
    constexpr SOCKET stop_sockfd{umock::sfd_base + 70};

    SSockaddr listenl_ssObj; // for getsockname() ipv6 lla return sockaddr&port
    listenl_ssObj =
        "[" + std::string(gIF_IPV6) + "]:" + std::to_string(LOCAL_PORT_V6);

    SSockaddr listeng_ssObj; // getsockname() ipv6 ula/gua return sockaddr&port
    listeng_ssObj = "[" + std::string(gIF_IPV6_ULA_GUA) +
                    "]:" + std::to_string(LOCAL_PORT_V6_ULA_GUA);

    SSockaddr stop_ssObj;
    stop_ssObj = "127.0.0.1:0";

    { // begin scope InSequence
        InSequence seq;

        // Provide a socket id for listening ipv6 lla.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET6, SOCK_STREAM, 0))
            .WillOnce(Return(listenl_sockfd));
        // Mock setsockopt() for ipv6 lla
        EXPECT_CALL(m_sys_socketObj, setsockopt(listenl_sockfd, _, _, _, _))
            .WillOnce(Return(0));
        // Provide a socket id for listening ipv6 ula/gua.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET6, SOCK_STREAM, 0))
            .WillOnce(Return(listeng_sockfd));
        // Mock setsockopt() for IPv6 ula/gua
        EXPECT_CALL(m_sys_socketObj, setsockopt(listeng_sockfd, _, _, _, _))
            .WillOnce(Return(0));

        // Bind socket to an ipv6 lla address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listenl_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on an ipv6 lla socket
        EXPECT_CALL(m_sys_socketObj, listen(listenl_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() from ipv6 lla address for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listenl_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in6))))))
            .WillOnce(DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(
                                &listenl_ssObj.ss)),
                            Return(0)));

        // Bind socket to an ipv6 ula/gua address for listening
        EXPECT_CALL(m_sys_socketObj, bind(listeng_sockfd, _, _))
            .WillOnce(Return(0));
        // Listen on an ipv6 ula/gua socket
        EXPECT_CALL(m_sys_socketObj, listen(listeng_sockfd, SOMAXCONN))
            .WillOnce(Return(0));
        // getsockname() from IPv6 ula/gua address for listening
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(listeng_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in6))))))
            .WillOnce(DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(
                                &listeng_ssObj.ss)),
                            Return(0)));

        // Provide a socket id for the stop socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
            .WillOnce(Return(stop_sockfd));
        // Bind socket to an ip address for the stop socket
        EXPECT_CALL(m_sys_socketObj, bind(stop_sockfd, _, _))
            .WillOnce(Return(0));
        // getsockname() for stop socket
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(stop_sockfd, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    sizeof(sockaddr_in))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&stop_ssObj.ss)),
                Return(0)));

        // Mock get_ssdp_sockets
        EXPECT_CALL(ssdpObj, get_ssdp_sockets(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [int get_ssdp_sockets()] "
                                   "MOCK MSG1089: Executing with success...\n"),
                      Return(UPNP_E_SUCCESS)));

        // Mock RunMiniServer
        EXPECT_CALL(miniserverObj, RunMiniServer(_))
            .WillOnce(
                DoAll(debugCoutMsg("UPnPlib [void RunMiniServer()] "
                                   "MOCK MSG1090: Executing with success...\n"),
                      set_gMServState(MSERV_RUNNING)));
    } // end scope InSequence

    // We need the threadpool to RunMiniServer().
    CThreadPoolInit tp(gMiniServerThreadPool);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);
}
#endif

TEST(StartMiniServerTestSuite, start_miniserver_already_running) {
    gMServState = MSERV_RUNNING;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR);
}

// Subroutine for multiple check of extended expectations.
void chk_minisocket(MiniServerSockArray& minisocket) {
    EXPECT_EQ(minisocket.miniServerSock4, INVALID_SOCKET);
    EXPECT_EQ(minisocket.miniServerPort4, 0u);
    EXPECT_EQ(minisocket.miniServerSock6, INVALID_SOCKET);
    EXPECT_EQ(minisocket.miniServerSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(minisocket.miniServerStopSock, INVALID_SOCKET);
    EXPECT_EQ(minisocket.ssdpSock4, INVALID_SOCKET);
    EXPECT_EQ(minisocket.ssdpSock6, INVALID_SOCKET);
    EXPECT_EQ(minisocket.ssdpSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(minisocket.stopPort, 0u);
    EXPECT_EQ(minisocket.miniServerPort6, 0u);
    EXPECT_EQ(minisocket.miniServerPort6UlaGua, 0u);
    EXPECT_EQ(minisocket.ssdpReqSock4, INVALID_SOCKET);
    EXPECT_EQ(minisocket.ssdpReqSock6, INVALID_SOCKET);
}

TEST(StartMiniServerTestSuite, get_miniserver_sockets_with_no_ip_addr) {
    // Using empty text ip addresses ("") will not bind to a valid socket.
    // TODO: With address 0, it would successful bind to all local ip
    // addresses. If this is intended then gIF_IPV4 should be set to "0.0.0.0",
    // or gIF_IPV6 to "::".

    // Due to unmocked bind() it returns successful with a valid ip address
    // instead of failing. That results in an endless loop with this test. So
    // we have to ensure empty ip addresses.
    gIF_IPV4[0] = '\0'; // Empty ip address
    gIF_IPV6[0] = '\0';
    gIF_IPV6_ULA_GUA[0] = '\0';
    LOCAL_PORT_V4 = 0;
    LOCAL_PORT_V6 = 0;
    LOCAL_PORT_V6_ULA_GUA = 0;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Using empty IPv4 address with disabled IPv6 stack must "
                     "not succeed.\n";
#ifndef UPNP_ENABLE_IPV6
        // This isn't relevant for new code because there is IPv6 always
        // available.
        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS); // Wrong!
#else
        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
#endif
    } else {

        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
    }

    // We do not get a valid socket with an empty text ip address.
    EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
    // It should return the 0 port.
    EXPECT_EQ(miniSocket.miniServerPort4, 0);
    {
        SCOPED_TRACE("");
        chk_minisocket(miniSocket);
    }
    // Close socket should fail, there is no valid socket.
    EXPECT_EQ(CLOSE_SOCKET_P(miniSocket.miniServerSock4), -1);
}

TEST_F(StartMiniServerMockFTestSuite,
       get_miniserver_sockets_with_one_ipv4_addr) {
    WINSOCK_INIT

    // Initialize needed structure
    constexpr SOCKET sockfd{umock::sfd_base + 71};
    SSockaddr saddrObj;
    saddrObj = "192.168.22.33:50080";
    // Get gIF_IPV4
    std::strcpy(gIF_IPV4, saddrObj.netaddr().c_str());
    LOCAL_PORT_V4 = saddrObj.get_port();
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Provide a socket file descriptor
    EXPECT_CALL(m_sys_socketObj, socket(saddrObj.ss.ss_family, SOCK_STREAM, 0))
        .WillOnce(Return(sockfd));

#ifdef UPNPLIB_WITH_NATIVE_PUPNP

    // Bind socket to an ip address
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    // Query port from socket
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(saddrObj.sizeof_ss())))))
        .WillOnce(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));
    // Listen on the socket
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, _)).WillOnce(Return(0));

#else // new code

    // Instantiate a socket object and point to it in miniSocket().
    upnplib::CSocket Sock4Obj(saddrObj.ss.ss_family, SOCK_STREAM);
    miniSocket.MiniSvrSock4Obj = &Sock4Obj;

    // Mock socket object initialization
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, _, _))
        .WillOnce(Return(0));
#ifdef _MSC_VER
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, _, _))
        .WillOnce(Return(0));
    // This option RESET! IPV6_V6ONLY
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, _, _))
        .WillOnce(Return(0));
#endif
    // Provide socket address from the socket file descriptor
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(saddrObj.sizeof_ss())))))
        .Times(g_dbug ? 3 : 2) // additional call with debug output
        .WillRepeatedly(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));
    EXPECT_CALL(m_sys_socketObj, getsockopt(sockfd, SOL_SOCKET, SO_TYPE, _, _))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(SOCK_STREAM), Return(0)));
    // Bind socket to an ip address
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    // Listen on the socket
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, _)).WillOnce(Return(0));
#endif

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    // EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerSock4, sockfd);
    // EXPECT_EQ(miniSocket.miniServerPort4, 0u);
    EXPECT_EQ(miniSocket.miniServerPort4, saddrObj.get_port());
    EXPECT_EQ(miniSocket.miniServerSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerStopSock, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.stopPort, 0u);
    EXPECT_EQ(miniSocket.miniServerPort6, 0u);
    EXPECT_EQ(miniSocket.miniServerPort6UlaGua, 0u);
    EXPECT_EQ(miniSocket.ssdpReqSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpReqSock6, INVALID_SOCKET);
}

TEST_F(StartMiniServerMockFTestSuite,
       get_miniserver_sockets_with_one_ipv6_lla_addr) {
    WINSOCK_INIT

    // Initialize needed structure
    constexpr SOCKET sockfd{umock::sfd_base + 10};
    SSockaddr saddrObj;
    saddrObj = "[fe80::fedc:0:0:3]:50079";
    // Get gIF_IPV6 and strip surounding brackets
    std::strcpy(gIF_IPV6, saddrObj.netaddr().c_str() + 1);
    gIF_IPV6[strlen(gIF_IPV6) - 1] = '\0';
    LOCAL_PORT_V6 = saddrObj.get_port();
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Provide a socket file descriptor
    EXPECT_CALL(m_sys_socketObj, socket(saddrObj.ss.ss_family, SOCK_STREAM, 0))
        .WillOnce(Return(sockfd));

#ifdef UPNPLIB_WITH_NATIVE_PUPNP

    // Mock setsockopt()
    EXPECT_CALL(m_sys_socketObj, setsockopt(sockfd, _, _, _, _))
        .WillOnce(Return(0));

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    // ===== result; should be same as with new code =====
    std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
              << ": Having only one IPv6 address should not fail.\n";
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET); // Wrong!
    EXPECT_EQ(miniSocket.miniServerSock6, INVALID_SOCKET);            // Wrong!
    EXPECT_EQ(miniSocket.miniServerPort6, 0);                         // Wrong!
}

#else // new_code

    // Instantiate a socket object and point to it in miniSocket().
    upnplib::CSocket Sock6LlaObj(saddrObj.ss.ss_family, SOCK_STREAM);
    miniSocket.MiniSvrSock6LlaObj = &Sock6LlaObj;

    // Mock socket object initialization
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, _, _))
        .WillOnce(Return(0));
#ifdef _MSC_VER
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, _, _))
        .WillOnce(Return(0));
#endif
    // This option RESET! IPV6_V6ONLY
    EXPECT_CALL(m_sys_socketObj,
                setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, _, _))
        .Times(2)
        .WillRepeatedly(Return(0));
    // Provide socket address from the socket file descriptor
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(saddrObj.sizeof_ss())))))
        .Times(g_dbug ? 3 : 2) // additional call with debug output
        .WillRepeatedly(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));
    EXPECT_CALL(m_sys_socketObj, getsockopt(sockfd, SOL_SOCKET, SO_TYPE, _, _))
        .WillOnce(DoAll(SetArgPtrIntValue<3>(SOCK_STREAM), Return(0)));
    // Bind socket to an ip address
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    // Listen on the socket
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, _)).WillOnce(Return(0));

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    // ===== result with fixes =====
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);
    EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerPort4, 0u);
    // EXPECT_EQ(miniSocket.miniServerSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerSock6, sockfd);
    EXPECT_EQ(miniSocket.miniServerSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerStopSock, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.stopPort, 0u);
    // EXPECT_EQ(miniSocket.miniServerPort6, 0u);
    EXPECT_EQ(miniSocket.miniServerPort6, saddrObj.get_port());
    EXPECT_EQ(miniSocket.miniServerPort6UlaGua, 0u);
    EXPECT_EQ(miniSocket.ssdpReqSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpReqSock6, INVALID_SOCKET);
}
#endif

TEST(StartMiniServerTestSuite, get_miniserver_sockets_with_invalid_ip_address) {
    strcpy(gIF_IPV4, "192.168.129.XXX"); // Invalid ip address
    gIF_IPV6[0] = '\0';
    gIF_IPV6_ULA_GUA[0] = '\0';
    LOCAL_PORT_V4 = 0;
    LOCAL_PORT_V6 = 0;
    LOCAL_PORT_V6_ULA_GUA = 0;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    if (old_code) {
        std::cout
            << CRED "[ BUG      ] " CRES << __LINE__
            << ": Using invalid IPv4 address with disabled IPv6 stack must "
               "not succeed.\n";
#ifndef UPNP_ENABLE_IPV6
        // This isn't relevant for new code because there is IPv6 always
        // available.
        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS); // Wrong!
#else
        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
#endif
    } else {

        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
    }
    {
        SCOPED_TRACE("");
        chk_minisocket(miniSocket);
    }
    // Close socket should fail because there is no socket to close.
    EXPECT_EQ(CLOSE_SOCKET_P(miniSocket.miniServerSock4), -1);
}

#ifdef _MSC_VER
TEST(StartMiniServerTestSuite, get_miniserver_sockets_uninitialized) {
    // MS Windows sockets are not initialized. Don't use WINSOCK_INIT. Unit
    // should never return a valid socket and/or port.
    CLogging logObj; // Output only with build type DEBUG.
    if (g_dbug)
        logObj.enable(UPNP_ALL);

    strcpy(gIF_IPV4, "192.168.200.199");
    gIF_IPV6[0] = '\0';
    gIF_IPV6_ULA_GUA[0] = '\0';

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);

    EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerPort4, 0);
    {
        SCOPED_TRACE("");
        chk_minisocket(miniSocket);
    }
    // Close socket
    EXPECT_EQ(CLOSE_SOCKET_P(miniSocket.miniServerSock4), -1);
}
#endif // _MSC_VER

TEST_F(StartMiniServerMockFTestSuite,
       get_miniserver_sockets_with_invalid_socket) {
    std::strcpy(gIF_IPV4, "192.168.12.9");
    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    InitMiniServerSockArray(&miniSocket);

    // Mock to get an invalid socket id
    EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(SetErrnoAndReturn(EINVAL, INVALID_SOCKET));

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    std::cout << CRED "[ BUG      ] " CRES << __LINE__
              << ": Getting an invalid socket for IPv4 address with "
                 "disabled IPv6 stack must not succeed.\n";
#ifndef UPNP_ENABLE_IPV6
    // This isn't relevant for new code because there is IPv6 always
    // available.
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS); // Wrong!
#else
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
#endif

#else // new code
      // Instantiate a socket object and point to it in miniSocket().
    upnplib::CSocket Sock4Obj(AF_INET, SOCK_STREAM);
    miniSocket.MiniSvrSock4Obj = &Sock4Obj;

#ifdef _MSC_VER
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError()).WillOnce(Return(WSAEINVAL));
#endif

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(&miniSocket, 0, 0, 0);

    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_OUTOF_SOCKET);
#endif
    {
        SCOPED_TRACE("");
        chk_minisocket(miniSocket);
    }
}

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
TEST(StartMiniServerTestSuite, init_socket_suff_successful) {
    WINSOCK_INIT

    // Set ip address and needed structure
    constexpr char text_addr[]{"2001:db8::3"};
    char addrbuf[INET6_ADDRSTRLEN];
    s_SocketStuff ss6;
    memset(&ss6, 0xAA, sizeof(ss6));

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss6, text_addr, 6), 0);

    EXPECT_EQ(ss6.ip_version, 6);
    EXPECT_STREQ(ss6.text_addr, text_addr);
    EXPECT_EQ(ss6.serverAddr6->sin6_family, AF_INET6);
    EXPECT_STREQ(inet_ntop(AF_INET6, &ss6.serverAddr6->sin6_addr, addrbuf,
                           sizeof(addrbuf)),
                 text_addr);
    // Valid real socket
    EXPECT_NE(ss6.fd, INVALID_SOCKET);
    EXPECT_EQ(ss6.try_port, 0);
    EXPECT_EQ(ss6.actual_port, 0);
    EXPECT_EQ(ss6.address_len,
              static_cast<socklen_t>(sizeof(*ss6.serverAddr6)));

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(
        ::getsockopt(ss6.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen), 0);
    EXPECT_FALSE(reuseaddr);

    // Close real socket
    EXPECT_EQ(CLOSE_SOCKET_P(ss6.fd), 0);
}

TEST(StartMiniServerTestSuite, init_socket_suff_reuseaddr) {
    // This isn't supported anymore.
    EXPECT_FALSE(MINISERVER_REUSEADDR);

    WINSOCK_INIT

    // Set ip address and needed structure
    constexpr char text_addr[]{"2001:db8::4"};
    s_SocketStuff ss6;

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss6, text_addr, 6), 0);

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(
        ::getsockopt(ss6.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen), 0);
    // The socket option should never be set.
    EXPECT_FALSE(reuseaddr);

    // Important! Otherwise repeated tests will fail later because all file
    // descriptors for the process are consumed.
    EXPECT_EQ(CLOSE_SOCKET_P(ss6.fd), 0) << std::strerror(errno);
}

TEST_F(StartMiniServerMockFTestSuite, init_socket_suff_with_invalid_socket) {
    // Set ip address and needed structure
    constexpr char text_addr[]{"192.168.99.85"};
    char addrbuf[INET_ADDRSTRLEN];
    s_SocketStuff ss4;

    // Mock to get an invalid socket id
    EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(SetErrnoAndReturn(EINVAL, INVALID_SOCKET));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAEINVAL));
#endif

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, sizeof(sockaddr_in));
    EXPECT_STREQ(inet_ntop(AF_INET, &ss4.serverAddr4->sin_addr, addrbuf,
                           sizeof(addrbuf)),
                 text_addr);
}

TEST(StartMiniServerTestSuite, init_socket_suff_with_invalid_ip_address) {
    WINSOCK_INIT
    // Set ip address and needed structure
    constexpr char text_addr[]{"2001:db8::1::2"}; // invalid ip address
    char addrbuf[INET6_ADDRSTRLEN];
    s_SocketStuff ss6;

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss6, text_addr, 6), 1);

    EXPECT_STREQ(ss6.text_addr, text_addr);
    EXPECT_EQ(ss6.fd, INVALID_SOCKET);
    EXPECT_EQ(ss6.ip_version, 6);
    EXPECT_EQ(ss6.serverAddr6->sin6_family, AF_INET6);
    EXPECT_EQ(ss6.address_len, sizeof(sockaddr_in6));
#ifdef _MSC_VER
    // Microsft Windows Visual Studio does not detect the invalid ip address.
    // Instead it cut the invalid part of the address.
    EXPECT_STREQ(inet_ntop(AF_INET6, &ss6.serverAddr6->sin6_addr, addrbuf,
                           sizeof(addrbuf)),
                 "2001:db8::1");
#else
    EXPECT_STREQ(inet_ntop(AF_INET6, &ss6.serverAddr6->sin6_addr, addrbuf,
                           sizeof(addrbuf)),
                 "::");
#endif
}

TEST(StartMiniServerTestSuite, init_socket_suff_with_invalid_ip_version) {
    // Set ip address and needed structure.
    constexpr char text_addr[]{"192.168.24.85"};
    char addrbuf[INET_ADDRSTRLEN];
    s_SocketStuff ss4;

    // Test Unit, arg<2> = 0 is an invalid ip version, must be 4 or 6.
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 0), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 0);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, 0);
    EXPECT_EQ(ss4.address_len, 0);
    EXPECT_STREQ(inet_ntop(AF_INET, &ss4.serverAddr4->sin_addr, addrbuf,
                           sizeof(addrbuf)),
                 "0.0.0.0");
}

TEST_F(StartMiniServerMockFTestSuite, do_bind_listen_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor sockfd
    // * Mocked bind() returns successful
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns a sockaddr with current ip address and
    //   port
    SSockaddr saddrObj;
    saddrObj = "192.168.54.188:50020";
    const char* text_addr = saddrObj.netaddr().c_str();
    char addrbuf[INET_ADDRSTRLEN];
    constexpr SOCKET sockfd{umock::sfd_base + 11};

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = reinterpret_cast<sockaddr*>(&s.ss);
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.actual_port = saddrObj.get_port();
    s.serverAddr4 = &saddrObj.sin;
    s.fd = sockfd;
    s.try_port = APPLICATION_LISTENING_PORT;
    s.address_len = sizeof(*s.serverAddr4);

    // If not mocked bind does not know the given ip address and fails.
    // The Unit will loop through all port numbers to find a free port
    // but will never find one. The program hungs in an endless loop.

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname(). It is the same address saddrObj with different port.
    saddrObj = APPLICATION_LISTENING_PORT;

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, SOMAXCONN)).WillOnce(Return(0));
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(saddrObj.sizeof_ss())))))
        .WillOnce(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_TRUE(saddrObj ==
                *reinterpret_cast<sockaddr_storage*>(s.serverAddr4));
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT + 1);
    EXPECT_EQ(s.actual_port, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.address_len, static_cast<socklen_t>(sizeof(*s.serverAddr4)));
}

TEST(StartMiniServerTestSuite, do_bind_listen_with_wrong_socket) {
    WINSOCK_INIT
    constexpr char text_addr[]{"0.0.0.0"};

    s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    EXPECT_EQ(CLOSE_SOCKET_P(s.fd), 0) << std::strerror(errno);
    // The socket fd wasn't got from a socket() call now and should trigger an
    // error.
    s.fd = umock::sfd_base + 39;
    s.try_port = 65534;

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND);
}

TEST_F(StartMiniServerMockFTestSuite, do_bind_listen_with_failed_listen) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor sockfd
    // * Mocked bind() returns successful
    // * Mocked listen() returns error

    SSockaddr saddrObj;
    saddrObj = "192.168.54.188";
    const char* text_addr = saddrObj.netaddr().c_str();
    constexpr SOCKET sockfd{umock::sfd_base + 12};

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.actual_port = saddrObj.get_port();
    s.serverAddr4 = &saddrObj.sin;
    s.fd = sockfd;
    s.try_port = APPLICATION_LISTENING_PORT;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, bind(s.fd, _, _)).WillOnce(Return(0));
    EXPECT_CALL(m_sys_socketObj, listen(s.fd, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EOPNOTSUPP, -1));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAEOPNOTSUPP));
#endif

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_LISTEN)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_LISTEN);

    // sock_close() is not needed because there is no socket called.
}

TEST_F(StartMiniServerMockFTestSuite, do_bind_listen_address_in_use) {
    // Configure expected system calls:
    // * Use fictive socket file descriptors
    // * Mocked bind() returns successful
    // * Mocked listen() returns error with errno EADDRINUSE
    // * Mocked getsockname() returns a sockaddr with current ip address and
    //   port

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should not loop through about 50000 ports to "
                     "find one free port.\n";
        // This very expensive behavior is skipped here and fixed in function
        // do_bind() of compa code.

    } else {

        SSockaddr saddrObj;
        saddrObj = "192.168.54.188:50024";
        const char* text_addr = saddrObj.netaddr().c_str();
        const in_port_t actual_port(saddrObj.get_port());
        char addrbuf[INET_ADDRSTRLEN];
        constexpr SOCKET sockfd_inuse{umock::sfd_base + 13};
        constexpr SOCKET sockfd_free{umock::sfd_base + 14};

        s_SocketStuff s;
        // Fill all fields of struct s_SocketStuff
        s.serverAddr = reinterpret_cast<sockaddr*>(&s.ss);
        s.ip_version = 4;
        s.text_addr = text_addr;
        s.serverAddr4->sin_family = saddrObj.ss.ss_family;
        s.actual_port = actual_port;
        s.serverAddr4->sin_port = saddrObj.sin.sin_port;
        s.serverAddr4->sin_addr = saddrObj.sin.sin_addr;
        s.fd = sockfd_inuse;
        s.try_port = actual_port + 1;
        s.address_len = saddrObj.sizeof_saddr();

        saddrObj = actual_port + 1; // To return by mock getsockname()

        // Mock system functions
        // A successful bind is expected but listen should fail with "address in
        // use"
        EXPECT_CALL(m_sys_socketObj, bind(sockfd_inuse, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(m_sys_socketObj, listen(sockfd_inuse, SOMAXCONN))
            .WillOnce(SetErrnoAndReturn(EADDRINUSE, SOCKET_ERROR));
#ifdef _MSC_VER
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAEADDRINUSE));
#endif
        // A second attempt will call init_socket_suff() to get a new socket.
        EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
            .WillOnce(Return(sockfd_free));
        EXPECT_CALL(m_sys_socketObj, bind(sockfd_free, _, _))
            .WillOnce(Return(0));
        EXPECT_CALL(m_sys_socketObj, listen(sockfd_free, SOMAXCONN))
            .WillOnce(Return(0));
        EXPECT_CALL(m_sys_socketObj,
                    getsockname(sockfd_free, _,
                                Pointee(Ge(static_cast<socklen_t>(
                                    saddrObj.sizeof_ss())))))
            .WillOnce(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));

        // Test Unit
        int ret_get_do_bind_listen = do_bind_listen(&s);
        EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SUCCESS)
            << errStrEx(ret_get_do_bind_listen, UPNP_E_SUCCESS);

        // Check all fields of struct s_SocketStuff
        EXPECT_EQ(s.ip_version, 4);
        EXPECT_STREQ(s.text_addr, text_addr);
        EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
        EXPECT_STREQ(inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf,
                               sizeof(addrbuf)),
                     text_addr);
        EXPECT_EQ(ntohs(s.serverAddr4->sin_port), actual_port + 1);
        EXPECT_EQ(s.fd, sockfd_free);
        EXPECT_EQ(s.try_port, actual_port + 2);
        EXPECT_EQ(s.actual_port, actual_port + 1);
        EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));
    }
}

TEST_F(StartMiniServerMockFTestSuite, bind_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Actual used port is 56789
    // * Next port to try is 56790
    // * Mocked bind() returns successful

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 15};
    constexpr char text_addr[]{"192.168.101.233"};
    char addrbuf[16];
    constexpr in_port_t actual_port{56789};
    constexpr in_port_t try_port{56790};

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = try_port;
    s.actual_port = actual_port;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));

    // Test Unit
    errno = EINVAL; // Check if this has an impact.
    int ret_do_bind = do_bind(&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
        << errStrEx(ret_do_bind, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), actual_port + 1);
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.try_port, try_port + 1);
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, actual_port); // Wrong!

    } else {

        EXPECT_EQ(s.actual_port, actual_port + 1);
    }

    // sock_close() is not needed because there is no socket called.
}

TEST_F(StartMiniServerMockFTestSuite, bind_with_invalid_argument) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Set actual used port
    // * Set next port to try
    // * Mocked bind() returns EINVAL

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 16};
    constexpr char text_addr[]{"192.168.202.233"};
    char addrbuf[16];
    constexpr in_port_t actual_port{50081};
    constexpr in_port_t try_port{50082};

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = try_port;
    s.actual_port = actual_port;
    s.address_len = sizeof(*s.serverAddr4);

    if (old_code) {

#ifdef _MSC_VER
        // We have calls in a loop with failed binding. See next note.
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .Times(2)
            .WillRepeatedly(Return(WSAEINVAL));
#endif
        // If bind() always returns failure due to unchanged invalid argument
        // the Unit will hung in an endless loop. There is no exit for this
        // condition. Here it will only stop after three loops because bind()
        // returns successful at last. This is fixed in new code.
        EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
            .WillOnce(SetErrnoAndReturn(EINVAL, -1))
            .WillOnce(SetErrnoAndReturn(EINVAL, -1))
            .WillOnce(Return(0));

        // Test Unit
        // This wrong condition is expected if the code wasn't fixed.
        int ret_do_bind = do_bind(&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
            << errStrEx(ret_do_bind, UPNP_E_SUCCESS); // Wrong!

    } else {

#ifdef _MSC_VER
        // The endless loop is fixed with new code, so we only have one call
        // for the error details.
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillRepeatedly(Return(WSAEINVAL));
#endif
        EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
            .WillOnce(SetErrnoAndReturn(EINVAL, -1));

        // Test Unit
        int ret_do_bind = do_bind(&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
            << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);
    }

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));

    if (old_code) {
        // See notes above about the endless loop. Expected values here are
        // meaningless and only tested to watch code changes.
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": do_bind() should never hung with testing all free ports "
               "before failing.\n";
        EXPECT_EQ(s.actual_port, actual_port);
        EXPECT_EQ(ntohs(s.serverAddr4->sin_port), actual_port + 3);
        EXPECT_EQ(s.try_port, try_port + 3);

    } else {

        EXPECT_EQ(s.actual_port, actual_port + 1);
        EXPECT_EQ(ntohs(s.serverAddr4->sin_port), actual_port + 1);
        EXPECT_EQ(s.try_port, try_port + 1);
    }
}

TEST_F(StartMiniServerMockFTestSuite, bind_with_try_port_number_overrun) {
    // This setup will 'try_port' overrun after 65535 to 0. The overrun should
    // finish the search for a free port to bind.
    //
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Set actual used port
    // * Set next port to try
    // * Mocked bind() returns always failure with errno EINVAL

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 17};
    constexpr char text_addr[]{"192.168.101.233"};
    char addrbuf[16];

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(56789);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 65533;
    s.actual_port = 56789;
    s.address_len = sizeof(*s.serverAddr4);

#ifdef _MSC_VER
    // errno is not supported with Winsock. I set it to an "old" serious value.
    // WSAGetLastError() must be used to get error details.
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
        .Times(3)
        .WillRepeatedly(SetErrnoAndReturn(ENOMEM, -1));
    // Here we get the right error detail.
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
        .WillRepeatedly(Return(WSAEADDRINUSE));
#else
    // errno will give the right error detail.
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
        .Times(3)
        .WillRepeatedly(SetErrnoAndReturn(EADDRINUSE, -1));
#endif

    // Test Unit
    int ret_do_bind = do_bind(&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Next try to bind a port should start with "
                     "APPLICATION_LISTENING_PORT but not with port 0.\n";
        EXPECT_EQ(s.try_port, 0); // Wrong!
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, 56789);

    } else {

        EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT);
        EXPECT_EQ(s.actual_port, 65535);
    }
}

TEST_F(StartMiniServerMockFTestSuite, bind_successful_with_two_tries) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Set actual used port
    // * Set next port to try
    // * Mocked bind() fails with two tries errno EADDRINUSE, then successful.

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 18};
    constexpr char text_addr[]{"192.168.101.233"};
    char addrbuf[16];
    s_SocketStuff s;

    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(56789);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 65533;
    s.actual_port = 56789;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
#ifdef _MSC_VER
    // errno is not supported with Winsock. I set it to "old" serious values.
    // WSAGetLastError() must be used to get error details.
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOMEM, -1))
        .WillOnce(SetErrnoAndReturn(EBADF, -1))
        // The system library never reset errno so don't do it here.
        // .WillOnce(SetErrnoAndReturn(0, 0));
        .WillOnce(Return(0));
    // Here we get the right error detail.
    EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
        .WillRepeatedly(Return(WSAEADDRINUSE));
#else
    // errno will give the right error detail.
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        // The system library never reset errno so don't do it here.
        // .WillOnce(SetErrnoAndReturn(0, 0));
        .WillOnce(Return(0));
#endif

    // Test Unit
    int ret_do_bind = do_bind(&s);

    if (old_code) {
        EXPECT_EQ(s.try_port, 0); // Wrong!

    } else {

        EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT);
    }

    EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
        << errStrEx(ret_do_bind, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, 56789);

    } else {

        EXPECT_EQ(s.actual_port, 65535);
    }
}

TEST(StartMiniServerTestSuite, bind_with_empty_parameter) {
    // With this test we have an initialized ip_version = 0, instead of valid 4
    // or 6. Switching for this value will never find an end.
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": This test would stuck the program in an endless loop.\n";

    } else {

        s_SocketStuff s{};
        s.serverAddr = (sockaddr*)&s.ss;

        // Test Unit
        int ret_do_bind = do_bind(&s);

        EXPECT_EQ(ret_do_bind, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);
    }
}

TEST(StartMiniServerTestSuite, bind_with_wrong_ip_version_assignment) {
    // Setting ip_version = 6 and sin_family = AF_INET and vise versa does not
    // fit. Provide needed data for the Unit.
    constexpr SOCKET sockfd{umock::sfd_base + 19};
    constexpr char text_addr[]{"192.168.101.233"};
    constexpr uint16_t try_port{65533};

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff.
    // Set ip_version = 6 and sin_family = AF_INET
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 6;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.actual_port = 56789;
    s.serverAddr4->sin_port = htons(s.actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = try_port;
    s.address_len = sizeof(*s.serverAddr4);

    if (old_code) {
        // Starting from try_port this will loop until 65535 with always the
        // same error. The short running test here is only given because we
        // start with try_port = 65533 so we have only three attempts here.
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": This should not loop through all free port numbers. It "
                     "will always fail.\n";
    }

    // Test Unit
    int ret_do_bind = do_bind(&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);

    // Set ip_version = 4 and sin_family = AF_INET6
    s.ip_version = 4;
    s.serverAddr4->sin_family = AF_INET6;
    s.try_port = try_port;

    // Test Unit
    ret_do_bind = do_bind(&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);
}

TEST_F(StartMiniServerMockFTestSuite, do_listen_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Actual used port 60000 will be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns successful

    // Provide needed data for the Unit
    SSockaddr saddrObj;
    saddrObj = "192.168.202.233:50083";
    const char* text_addr = saddrObj.netaddr().c_str();
    constexpr SOCKET sockfd{umock::sfd_base + 20};
    char addrbuf[INET_ADDRSTRLEN];
    constexpr in_port_t try_port{0}; // not used

    s_SocketStuff s{};
    // Fill needed fields of struct s_SocketStuff
    s.serverAddr = reinterpret_cast<sockaddr*>(&s.ss);
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = saddrObj.ss.ss_family;
    s.serverAddr4->sin_addr = saddrObj.sin.sin_addr;
    s.fd = sockfd;
    s.address_len = saddrObj.sizeof_saddr();

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, SOMAXCONN)).WillOnce(Return(0));
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(saddrObj.sizeof_ss())))))
        .WillOnce(DoAll(SetArgPointee<1>(saddrObj.sa), Return(0)));

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_do_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, saddrObj.get_port());
    EXPECT_EQ(s.try_port, try_port); // not used
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));
}

TEST_F(StartMiniServerMockFTestSuite, do_listen_not_supported) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Actual used port will not be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns with EOPNOTSUPP
    // * Mocked getsockname() is not called

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 21};
    constexpr char text_addr[] = "192.168.101.203";
    char addrbuf[16];

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = 0; // not used
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 0; // not used
    s.actual_port = 0;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EOPNOTSUPP, -1));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAEOPNOTSUPP));
#endif

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_LISTEN)
        << errStrEx(ret_do_listen, UPNP_E_LISTEN);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 0);
    EXPECT_EQ(s.try_port, 0); // not used
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));
}

TEST_F(StartMiniServerMockFTestSuite, do_listen_insufficient_resources) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Actual used port will not be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns with ENOBUFS

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 22};
    constexpr char text_addr[] = "192.168.101.203";
    char addrbuf[16];

    s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = 0; // not used
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 0; // not used
    s.actual_port = 0;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, listen(sockfd, SOMAXCONN)).WillOnce(Return(0));
    EXPECT_CALL(m_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, -1));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAENOBUFS));
#endif

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_do_listen, UPNP_E_INTERNAL_ERROR);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(
        inet_ntop(AF_INET, &s.serverAddr4->sin_addr, addrbuf, sizeof(addrbuf)),
        text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 0);
    EXPECT_EQ(s.try_port, 0); // not used
    EXPECT_EQ(s.address_len, (socklen_t)sizeof(*s.serverAddr4));
}
#endif

TEST_F(StartMiniServerMockFTestSuite, get_port_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Set actual socket used port
    // * Mocked getsockname() returns successful
    WINSOCK_INIT

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 23};
    constexpr char text_addr[] = "192.168.154.188";
    constexpr uint16_t actual_port{55555};
    // This is for the returned port number
    uint16_t port{0xAAAA};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    CAddrinfo ai(std::string(text_addr), std::to_string(actual_port), AF_INET,
                 SOCK_STREAM, AI_NUMERICHOST | AI_NUMERICSERV);
    ai.init();

    // Mock system functions
    EXPECT_CALL(
        m_sys_socketObj,
        getsockname(sockfd, _,
                    Pointee(Ge(static_cast<socklen_t>(ai->ai_addrlen)))))
        .WillOnce(
            DoAll(SetArgPointee<1>(*ai->ai_addr),
                  SetArgPointee<2>(static_cast<socklen_t>(ai->ai_addrlen)),
                  Return(0)));


    // Test Unit
    EXPECT_EQ(get_port(sockfd, &port), 0);

    EXPECT_EQ(port, actual_port);
}

TEST_F(StartMiniServerMockFTestSuite, get_port_wrong_sockaddr_family) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Mocked getsockname() returns successful unusable sockaddr family 0.

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 24};
    // This is for the returned port number
    uint16_t port{0xAAAA};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname(). It is empty.
    const sockaddr sa{};

    // Mock system function
    EXPECT_CALL(m_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(sa),
                        SetArgPointee<2>((socklen_t)sizeof(sa)), Return(0)));

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Getting port number with unusable sockaddr family must "
                     "fail.\n";
        EXPECT_EQ(get_port(sockfd, &port), 0); // Wrong!

    } else {

        EXPECT_EQ(get_port(sockfd, &port), -1);
    }

    // The port variable should not be modified.
    EXPECT_EQ(port, 0xAAAA);
}

TEST_F(StartMiniServerMockFTestSuite, get_port_fails) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor
    // * Mocked getsockname() fails with insufficient resources (ENOBUFS).

    // Provide needed data for the Unit
    constexpr SOCKET sockfd{umock::sfd_base + 25};
    // This is for the returned port number
    uint16_t port{0xAAAA};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname(). It will be empty.
    const sockaddr sa{};

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(sa), SetErrnoAndReturn(ENOBUFS, -1)));

    // Test Unit
    EXPECT_EQ(get_port(sockfd, &port), -1);

    EXPECT_EQ(errno, ENOBUFS);
    EXPECT_EQ(port, 0xAAAA);
}

TEST(StartMiniServerTestSuite, get_miniserver_stopsock) {
    // Here we test a real connection to the loopback device. This needs
    // initialization of sockets on MS Windows. We also have to close the
    // socket.
    WINSOCK_INIT
    MiniServerSockArray out;
    InitMiniServerSockArray(&out);

    // Test Unit
    int ret_get_miniserver_stopsock = get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SUCCESS);

    EXPECT_NE(out.miniServerStopSock, (SOCKET)0);
    EXPECT_NE(out.stopPort, 0);
    EXPECT_EQ(out.stopPort, miniStopSockPort);

    // Get socket object from the bound socket
    CSocket_basic sockObj(out.miniServerStopSock);
    sockObj.init();

    // and verify its settings
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_port(), miniStopSockPort);
    EXPECT_EQ(sockObj.netaddr(), "127.0.0.1");

    // Close socket
    EXPECT_EQ(sock_close(out.miniServerStopSock), 0);
}

TEST_F(StartMiniServerMockFTestSuite, get_miniserver_stopsock_fails) {
    // Configure expected system calls:
    // * Get a socket() fails with EACCES (Permission denied).
    // * bind() is not called.
    // * getsockname() is not called.

    // Provide needed data for the Unit
    MiniServerSockArray out;
    InitMiniServerSockArray(&out);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(SetErrnoAndReturn(EACCES, -1));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSAEACCES));
#endif

    // Test Unit
    int ret_get_miniserver_stopsock = get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_OUTOF_SOCKET);

    // Close socket; we don't need to close a mocked socket
}

TEST_F(StartMiniServerMockFTestSuite, get_miniserver_stopsock_bind_fails) {
    // Configure expected system calls:
    // * socket() returns file descriptor.
    // * bind() fails with ENOMEM.
    // * getsockname() is not called.

    // Provide needed data for the Unit
    MiniServerSockArray out;
    InitMiniServerSockArray(&out);
    const SOCKET sockfd{umock::sfd_base + 26};

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(sockfd));
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOMEM, -1));
#ifdef _MSC_VER
    if (!old_code)
        EXPECT_CALL(m_winsock2Obj, WSAGetLastError())
            .WillOnce(Return(WSA_NOT_ENOUGH_MEMORY));
#endif

    // Test Unit
    int ret_get_miniserver_stopsock = get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SOCKET_BIND);

    // Close socket; we don't need to close a mocked socket
}

TEST_F(StartMiniServerMockFTestSuite,
       get_miniserver_stopsock_getsockname_fails) {
    // Configure expected system calls:
    // * socket() returns file descriptor.
    // * bind() returns successful.
    // * getsockname() fails with ENOBUFS (Cannot allocate memory).

    // Provide needed data for the Unit
    MiniServerSockArray out;
    InitMiniServerSockArray(&out);
    const SOCKET sockfd{umock::sfd_base + 27};

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(sockfd));
    EXPECT_CALL(m_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    EXPECT_CALL(m_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, -1));

    // Test Unit
    int ret_get_miniserver_stopsock = get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_INTERNAL_ERROR);

    // Close socket; we don't need to close a mocked socket
}

} // namespace utest


int main(int argc, char** argv) {
#ifdef _MSC_VER
    // Uninitialize Windows sockets because it is global initialized with using
    // the upnplib library. We need this for testing uninitialized sockets.
    ::WSACleanup();
#endif
    ::testing::InitGoogleMock(&argc, argv);
#include "utest/utest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
