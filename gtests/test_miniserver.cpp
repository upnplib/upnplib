// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-07-13

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS upnplib
#endif

#include "pupnp/upnp/src/genlib/miniserver/miniserver.cpp"
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
#include "core/src/net/miniserver.cpp"
#endif

#include "upnplib/upnptools.hpp" // For upnplib_native only
#include "upnplib/sock.hpp"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Gt;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;
using ::testing::ThrowsMessage;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
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

StartMiniServer() has started RunMiniServer() as thread and it has set a
miniserver stopsock(). RunMiniServer() polls receive_from_stopSock() for an
incomming message "ShutDown" send to stopsock bound to "127.0.0.1".

   RunMiniServer()
   |__ block until receive_from_stopSock()
   |__ sock_close()
   |__ free()
*/

//
// Mocked system calls
// ===================
class Mock_sys_socket : public Bsys_socket {
    // Class to mock the free system functions.
    Bsys_socket* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_socket() {
        m_oldptr = sys_socket_h;
        sys_socket_h = this;
    }
    virtual ~Mock_sys_socket() override { sys_socket_h = m_oldptr; }

    MOCK_METHOD(int, socket,
                (int domain, int type, int protocol),
                (override));
    MOCK_METHOD(int, bind,
                (int sockfd, const struct sockaddr* addr, socklen_t addrlen),
                (override));
    MOCK_METHOD(int, listen,
                (int sockfd, int backlog),
                (override));
    MOCK_METHOD(int, accept,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(UPNPLIB_SIZE_T_INT, recvfrom,
                (int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(int, setsockopt,
                (int sockfd, int level, int optname, const char* optval,
                 socklen_t optlen),
                (override));
    MOCK_METHOD(int, getsockname,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
};

class Mock_sys_select : public Bsys_select {
    // Class to mock the free system functions.
    Bsys_select* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_select() {
        m_oldptr = sys_select_h;
        sys_select_h = this;
    }
    virtual ~Mock_sys_select() override { sys_select_h = m_oldptr; }

    MOCK_METHOD(int, select,
                (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
                 struct timeval* timeout),
                (override));
};

class Mock_stdlib : public Bstdlib {
    // Class to mock the free system functions.
    Bstdlib* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_stdlib() { m_oldptr = stdlib_h; stdlib_h = this; }
    virtual ~Mock_stdlib() { stdlib_h = m_oldptr; }

    MOCK_METHOD(void, free, (void* ptr), (override));
};
// clang-format on

//
// Custom action to return a string literal
// Source: https://groups.google.com/g/googlemock/c/lQqCMW1ANQA
// simple version: ACTION_P(StrCpyToArg0, str) { strcpy(arg0, str); }
ACTION_TEMPLATE(StrCpyToArg, HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(str)) {
    strcpy(std::get<k>(args), str);
}

// This test uses real connections and isn't portable. It is only for humans to
// see how it works and should not always enabled.
#if 0
TEST(StartMiniServerTestSuite, StartMiniServer_in_context) {
    // Debug logging only works if compiled with build type DEBUG
    UpnpSetLogLevel(UPNP_INFO);
    int ret_UpnpInitLog = UpnpInitLog();
    EXPECT_EQ(ret_UpnpInitLog, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitLog, UPNP_E_SUCCESS);

    // MINISERVER_REUSEADDR = false;
    // gIF_IPV4 = "";
    // LOCAL_PORT_V4 = 0;
    // LOCAL_PORT_V6 = 0;
    // LOCAL_PORT_V6_ULA_GUA = 0;

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
    ASSERT_FALSE(MINISERVER_REUSEADDR);
    ASSERT_EQ(UpnpSdkInit, 0);
    ASSERT_STRNE(gIF_IPV4, "");
    UpnpSdkInit = 1;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);

    ASSERT_FALSE(MINISERVER_REUSEADDR);
    EXPECT_EQ(UpnpSdkInit, 1);
    EXPECT_EQ(LOCAL_PORT_V4, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(LOCAL_PORT_V6, 0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, 0);

    EXPECT_EQ(StopMiniServer(), 0); // Always returns 0

    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);

    UpnpCloseLog();
}
#endif

TEST(StartMiniServerTestSuite, start_miniserver_already_running) {
    MINISERVER_REUSEADDR = false;
    gMServState = MSERV_RUNNING;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR);
}

TEST(StartMiniServerTestSuite, get_miniserver_sockets) {
    MINISERVER_REUSEADDR = false;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    if (old_code) {
        // This is a bug and fixed in new_code.
        EXPECT_NE(miniSocket.miniServerSock4, INVALID_SOCKET);
        EXPECT_EQ(miniSocket.miniServerPort4, APPLICATION_LISTENING_PORT);
    } else {
        EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
        EXPECT_EQ(miniSocket.miniServerPort4, 0);
    }
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

    // Close socket
    if (old_code)
        // This is a bug. Due to a wrong ip address there shouldn't be an open
        // socket that can be closed. This is fixed in new_code.
        EXPECT_EQ(UPNPLIB_CLOSE_SOCKET(miniSocket.miniServerSock4), 0);
    else
        EXPECT_EQ(UPNPLIB_CLOSE_SOCKET(miniSocket.miniServerSock4), -1);
}

TEST(StartMiniServerTestSuite, get_miniserver_sockets_with_invalid_socket) {
    MINISERVER_REUSEADDR = false;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    ::InitMiniServerSockArray(&miniSocket);

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Mock to get an invalid socket id
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(INVALID_SOCKET));

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        ::get_miniserver_sockets(&miniSocket, 0, 0, 0);
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);

    EXPECT_EQ(miniSocket.miniServerSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerStopSock, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpSock6UlaGua, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.stopPort, 0u);

    EXPECT_EQ(miniSocket.miniServerPort4, 0);

    EXPECT_EQ(miniSocket.miniServerPort6, 0u);
    EXPECT_EQ(miniSocket.miniServerPort6UlaGua, 0u);
    EXPECT_EQ(miniSocket.ssdpReqSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.ssdpReqSock6, INVALID_SOCKET);

    // Close socket
    EXPECT_EQ(UPNPLIB_CLOSE_SOCKET(miniSocket.miniServerSock4), -1);
}

TEST(StartMiniServerTestSuite, init_socket_suff) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.54.85"};
    struct s_SocketStuff ss4;
    memset(&ss4, 0xAA, sizeof(ss4));

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);

    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), text_addr);
    // Valid real socket
    EXPECT_NE(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.try_port, 0);
    EXPECT_EQ(ss4.actual_port, 0);
    EXPECT_EQ(ss4.address_len, sizeof(*ss4.serverAddr4));

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(getsockopt(ss4.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen),
              0);
    EXPECT_FALSE(reuseaddr);

    // Close real socket
    EXPECT_EQ(UPNPLIB_CLOSE_SOCKET(ss4.fd), 0);
}

TEST(StartMiniServerTestSuite, init_socket_suff_reuseaddr) {
    MINISERVER_REUSEADDR = true;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(getsockopt(ss4.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen),
              0);
    EXPECT_TRUE(reuseaddr);
}

TEST(StartMiniServerTestSuite, init_socket_suff_with_invalid_socket) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.99.85"};
    struct s_SocketStuff ss4;

    // Mock to get an invalid socket id
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(INVALID_SOCKET));

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, 16);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "192.168.99.85");
}

TEST(StartMiniServerTestSuite, init_socket_suff_with_invalid_ip_address) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"9999.9999.999.99"};
    struct s_SocketStuff ss4;

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Test Unit, needs initialized sockets on MS Windows
    if (old_code) {
        std::cout << "  BUG! With an invalid ip address the function call "
                     "should fail.\n";
        EXPECT_EQ(
            ::init_socket_suff((struct ::s_SocketStuff*)&ss4, text_addr, 4), 0);
        EXPECT_STREQ(ss4.text_addr, text_addr);
        EXPECT_GE(ss4.fd, 3);

    } else {

        EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 1);
        EXPECT_STREQ(ss4.text_addr, text_addr);
        EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    }

    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, 16);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "0.0.0.0");
}

TEST(StartMiniServerTestSuite, init_socket_suff_with_invalid_ip_version) {
    // Set ip address and needed structure
    const char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 0), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 0);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, 0);
    EXPECT_EQ(ss4.address_len, 0);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "0.0.0.0");
}

TEST(StartMiniServerTestSuite, do_bind_listen) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 600
    // * Mocked bind() returns successful
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns a sockaddr with current ip address and
    //   port

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};

    struct s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    s.fd = 600;
    s.try_port = APPLICATION_LISTENING_PORT;

    // BUG! If not mocked bind does not know the given ip address and fails.
    // The Unit will loop through all port numbers to find a free port
    // but will never find one. The program hangs in an endless loop.

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    struct sockaddr* sa{(sockaddr*)&ss};
    struct sockaddr_in* sa_in{(sockaddr_in*)&ss};
    sa_in->sin_family = AF_INET;
    sa_in->sin_port = htons(APPLICATION_LISTENING_PORT);
    inet_pton(AF_INET, text_addr, &sa_in->sin_addr);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(600, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(600, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, getsockname(600, _, Pointee(sizeof(ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sa), Return(0)));

    // Test Unit
    EXPECT_EQ(do_bind_listen(&s), 0);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.fd, 600);
    EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT + 1);
    EXPECT_EQ(s.actual_port, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(StartMiniServerTestSuite, do_bind_listen_with_wrong_socket) {
    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"0.0.0.0"};

    struct s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    // The socket id wasn't got from a socket() call and should trigger an
    // error.
    s.fd = 32000;
    s.try_port = 65534;

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND);
}

TEST(StartMiniServerTestSuite, do_bind_listen_with_failed_listen) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 600
    // * Mocked bind() returns successful
    // * Mocked listen() returns error

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};

    struct s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    s.fd = 600;
    s.try_port = APPLICATION_LISTENING_PORT;

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(600, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(600, SOMAXCONN))
        .WillOnce(Return(SOCKET_ERROR));

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_LISTEN)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_LISTEN);
}

TEST(StartMiniServerTestSuite, do_bind_listen_address_in_use) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 600
    // * Mocked bind() returns successful
    // * Mocked listen() returns error with errno EADDRINUSE

    GTEST_SKIP() << "  # Test needs rework when do_listen() is improved.";

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};

    struct SockAddr sock;
    sock.addr_set(text_addr, APPLICATION_LISTENING_PORT);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(600))  // This socket is already listening
        .WillOnce(Return(601)); // This socket is free
    // First attempt to listen to a socket will fail
    EXPECT_CALL(mocked_sys_socketObj, bind(600, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(600, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, SOCKET_ERROR));
    // Second attempt to listen to another socket will succeed
    EXPECT_CALL(mocked_sys_socketObj, bind(601, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(601, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(601, _, Pointee(sizeof(sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    struct s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    s.try_port = APPLICATION_LISTENING_PORT + 1;

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, 601);
    EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT + 2);
    EXPECT_EQ(s.actual_port, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(DoBindTestSuite, bind_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 511
    // * Actual used port is 56789
    // * Next port to try is 56790
    // * Mocked bind() returns successful

    // Provide needed data for the Unit
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    struct ::s_SocketStuff s;

    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(56789);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 56790;
    s.actual_port = 56789;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _)).Times(1);

    // Test Unit
    errno = EINVAL; // Check if this has an impact.
    int ret_do_bind = do_bind((s_SocketStuff*)&s);

    EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
        << errStrEx(ret_do_bind, UPNP_E_SUCCESS);
}

#if 0
TEST(DoBindTestSuite, bind_with_try_port_overrun) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";
#ifdef _WIN32
    if (old_code)
        GTEST_SKIP() << "  # BUG! This test fails on MS Windows due to "
                        "checking wrong error conditions.";
#endif

    // Configure expected system calls:
    // * Use fictive socket file descriptor 511
    // * Actual used port is 56789
    // * Next port to try is 65533
    // * Mocked bind() returns always failure with errno EINVAL
    //
    // This setup will 'try_port' overrun after 65535 to 0. The overrun should
    // finish the search for a free port to bind.

    // Provide needed data for the Unit
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    struct ::s_SocketStuff s;

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

    // Mock system function, must also set errno
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
        .Times(3)
        .WillRepeatedly(SetErrnoAndReturn(EINVAL, -1));

    // Test Unit
    int ret_do_bind;
    if (old_code)
        ret_do_bind = ::do_bind((::s_SocketStuff*)&s);
    else
        ret_do_bind = upnplib::do_bind((upnplib::s_SocketStuff*)&s);

    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 56789);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout << "  BUG! Next try to bind a port should start with "
                     "APPLICATION_LISTENING_PORT but not with port 0.\n";
        EXPECT_EQ(s.try_port, 0);

    } else {

        EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT);
    }
}

TEST(DoBindTestSuite, bind_successful_with_two_tries) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 511
    // * Actual used port is 56789
    // * Next port to try is 65533
    // * Mocked bind() fails with two tries errno EADDRINUSE, then successful.

    // Provide needed data for the Unit
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    struct ::s_SocketStuff s;

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

    // Mock system function, must also set errno
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        // The system library never reset errno so don't do it here.
        // .WillOnce(SetErrnoAndReturn(0, 0));
        .WillOnce(Return(0));

    // Test Unit
    int ret_do_bind;
    if (old_code) {
        ret_do_bind = ::do_bind((::s_SocketStuff*)&s);
        EXPECT_EQ(s.try_port, 0);
    } else {
        ret_do_bind = upnplib::do_bind((upnplib::s_SocketStuff*)&s);
        EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT);
    }
    EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
        << errStrEx(ret_do_bind, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 56789);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(DoBindTestSuite, bind_with_empty_parameter) {
    // With this test we have an initialized ip_version = 0, instead of valid 4
    // or 6. Switching for this value will never find an end.
    if (old_code)
        GTEST_SKIP()
            << "  # BUG! This test stuck the program in an endless loop.";

    struct ::s_SocketStuff s {};
    s.serverAddr = (struct sockaddr*)&s.ss;

    // Test Unit
    int ret_do_bind;
    ret_do_bind = upnplib::do_bind((upnplib::s_SocketStuff*)&s);

    EXPECT_EQ(ret_do_bind, UPNP_E_INVALID_PARAM)
        << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);
}

TEST(DoBindTestSuite, bind_with_wrong_ip_version_assignment) {
    if (!old_code)
        GTEST_SKIP() << "             upnplib::do_bind() needs rework";

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Setting ip_version = 6 and sin_family = AF_INET and vise versa does not
    // fit. Provide needed data for the Unit.
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    struct ::s_SocketStuff s;

    // Fill all fields of struct s_SocketStuff.
    // Set ip_version = 6 and sin_family = AF_INET
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 6;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(56789);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = 65535;
    s.actual_port = 56789;
    s.address_len = sizeof(*s.serverAddr4);

    // Test Unit
    int ret_do_bind;
    if (old_code) {
        std::cout << "  BUG! Should be UPNP_E_INVALID_PARAM(-101), but not "
                     "UPNP_E_SOCKET_BIND(-203).\n";
        ret_do_bind = ::do_bind((::s_SocketStuff*)&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
            << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);

    } else {

        ret_do_bind = upnplib::do_bind((upnplib::s_SocketStuff*)&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);
    }

    // Set ip_version = 4 and sin_family = AF_INET6
    s.ip_version = 4;
    s.serverAddr4->sin_family = AF_INET6;
    s.try_port = 65535;

    // Test Unit
    if (old_code) {
        ret_do_bind = ::do_bind((::s_SocketStuff*)&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
            << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);

    } else {

        ret_do_bind = upnplib::do_bind((upnplib::s_SocketStuff*)&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_INVALID_PARAM)
            << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);
    }
}

TEST(StartMiniServerTestSuite, do_listen) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 512
    // * Actual used port is 60000
    // * Next port to try is 60001
    // * Mocked bind() returns successful
    // * Mocked getsockname() returns successful

    // Provide needed data for the Unit
    const char text_addr[] = "192.168.202.233";
    struct s_SocketStuff s;
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = htons(60000);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = 512;
    s.try_port = 60001;
    s.actual_port = 60000;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, listen(512, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, getsockname(512, _, _)).Times(1);

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_do_listen, UPNP_E_SUCCESS);
}

TEST(StartMiniServerTestSuite, get_port) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 1000
    // * Actual used port is 55555
    // * Mocked getsockname() returns successful

    // This is for the returned port number
    uint16_t port{0};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct sockaddr sa;
    memset(&sa, 0, sizeof(sa));
    struct sockaddr_in* sa_in{(sockaddr_in*)&sa};
    sa_in->sin_family = AF_INET;
    sa_in->sin_port = htons(55555);
    inet_pton(AF_INET, "192.168.154.188", &sa_in->sin_addr);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(1000, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(sa), Return(0)));

    // Test Unit
    EXPECT_EQ(get_port(1000, &port), 0);

    EXPECT_EQ(port, 55555);
}

TEST(StartMiniServerTestSuite, get_miniserver_stopsock) {
    MiniServerSockArray out{};
    ::InitMiniServerSockArray(&out);

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Get stop socket, needs initialized sockets on MS Windows
    int ret_get_miniserver_stopsock = ::get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SUCCESS);

    EXPECT_NE(out.miniServerStopSock, 0);
    EXPECT_NE(out.stopPort, 0);
    EXPECT_EQ(out.stopPort, ::miniStopSockPort);

    // Provide a sockaddr structure to getsockname().
    struct sockaddr_storage ss;
    socklen_t sslen = sizeof(ss);
    memset(&ss, 0, sslen);
    struct sockaddr* sa{(sockaddr*)&ss};
    struct sockaddr_in* sa_in{(sockaddr_in*)&ss};

    // Get address information direct from the bind socket
    ASSERT_EQ(getsockname(out.miniServerStopSock, sa, &sslen), 0);
    // and verify its settings
    EXPECT_EQ(sa_in->sin_family, AF_INET);
    char text_addr[INET_ADDRSTRLEN];
    EXPECT_EQ(ntohs(sa_in->sin_port), ::miniStopSockPort);
    ASSERT_NE(
        inet_ntop(AF_INET, &sa_in->sin_addr, text_addr, sizeof(text_addr)),
        nullptr);
    EXPECT_STREQ(text_addr, "127.0.0.1");
}

TEST(RunMiniServerTestSuite, receive_from_stopSock) {
    MiniServerSockArray serverSockArray;
    ::InitMiniServerSockArray(&serverSockArray);

#ifdef _WIN32
    // Initialize sockets
    WSADATA wsaData;
    ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), NO_ERROR);
#endif

    // Get stop socket, needs initialized sockets on MS Windows
    int ret_get_miniserver_stopsock =
        ::get_miniserver_stopsock(&serverSockArray);
    ASSERT_EQ(ret_get_miniserver_stopsock, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SUCCESS);

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(serverSockArray.miniServerStopSock, &rdSet);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    struct SockAddr sock;
    sock.addr_set("192.168.167.166", 54321);

    EXPECT_CALL(mocked_sys_socketObj,
                recvfrom(serverSockArray.miniServerStopSock, _, 25, 0, _,
                         Pointee(sizeof(sockaddr_storage))))
        .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"),
                        SetArgPointee<4>(*sock.addr), Return(8)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    EXPECT_TRUE(
        ::receive_from_stopSock(serverSockArray.miniServerStopSock, &rdSet));
}
#endif

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
}
