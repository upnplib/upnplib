// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-25

#include "pupnp/upnp/src/genlib/miniserver/miniserver.cpp"
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
#include "core/src/net/miniserver.cpp"
#endif

#include "upnplib/upnptools.hpp" // For upnplib_native only
#include "upnplib/port.hpp"
#include "upnplib/socket.hpp"
#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;

using ::upnplib::testing::CaptureStdOutErr;
using ::upnplib::testing::ContainsStdRegex;
using ::upnplib::testing::StrCpyToArg;

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS upnplib
#endif

class CLogging { /*
 * Use it for example with:
    class CLogging loggingObj; // Output only with build type DEBUG.
* or
    class CLogging loggingObj(UPNP_ALL); // Output only with build type DEBUG.
 * or other loglevel.
 */
  public:
    CLogging(Upnp_LogLevel a_loglevel = UPNP_ALL) {
        UpnpSetLogLevel(a_loglevel);
        if (UpnpInitLog() != UPNP_E_SUCCESS) {
            throw std::runtime_error(std::string(
                "UpnpInitLog(): failed to initialize pupnp logging."));
        }
    }

    ~CLogging() { UpnpCloseLog(); }
};

//
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

StartMiniServer() has started RunMiniServer() as thread. It then waits blocked
with select() undefinetly (timeout set to NULL) until it receive from any
selected socket. StartMiniServer() also has set a miniserver stopsock() on
another thread. This thread polls receive_from_stopSock() for an incomming
message "ShutDown" send to stopsock bound to "127.0.0.1". This will be recieved
by select() so it will always enable a blocked (waiting) select().

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
    MOCK_METHOD(size_t, recvfrom,
                (int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(int, getsockopt,
                (int sockfd, int level, int optname, void* optval, socklen_t* optlen),
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

class Mock_unistd : public Bunistd {
    // Class to mock the free system functions.
    Bunistd* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_unistd() { m_oldptr = unistd_h; unistd_h = this; }
    virtual ~Mock_unistd() override { unistd_h = m_oldptr; }

    MOCK_METHOD(int, PUPNP_CLOSE_SOCKET, (PUPNP_SOCKET_TYPE fd), (override));
};
// clang-format on

//
// Miniserver TestSuite
// ====================
class MiniServerFTestSuite : public ::testing::Test {
#ifdef _WIN32
    // Initialize and cleanup Windows sochets
  protected:
    MiniServerFTestSuite() {
        WSADATA wsaData;
        int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc != NO_ERROR) {
            throw std::runtime_error(
                std::string("Failed to start Windows sockets (WSAStartup)."));
        }
    }

    ~MiniServerFTestSuite() override { WSACleanup(); }
#endif
};

typedef MiniServerFTestSuite StartMiniServerFTestSuite;
typedef MiniServerFTestSuite DoBindFTestSuite;
typedef MiniServerFTestSuite StopMiniServerFTestSuite;
typedef MiniServerFTestSuite RunMiniServerFTestSuite;

//
// This test uses real connections and isn't portable. It is only for humans to
// see how it works and should not always enabled.
#if 0
TEST(StartMiniServerTestSuite, StartMiniServer_in_context) {
    // MINISERVER_REUSEADDR = false;
    // gIF_IPV4 = "";
    // LOCAL_PORT_V4 = 0;
    // LOCAL_PORT_V6 = 0;
    // LOCAL_PORT_V6_ULA_GUA = 0;

    // class Mock_sys_socket mocked_sys_socketObj;
    // class Mock_sys_select mocked_sys_selectObj;
    // class Mock_stdlib  mocked_stdlibObj;
    // class Mock_unistd mocked_unistdObj;

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
}
#endif

//
TEST(StartMiniServerTestSuite, start_miniserver_already_running) {
    MINISERVER_REUSEADDR = false;
    gMServState = MSERV_RUNNING;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR);
}

TEST_F(StartMiniServerFTestSuite, get_miniserver_sockets) {
    // Initialize needed structure
    MINISERVER_REUSEADDR = false;
    strcpy(gIF_IPV4, "192.168.245.254");
    const int sockfd{333};
    struct SockAddr sock;
    sock.addr_set(gIF_IPV4, APPLICATION_LISTENING_PORT);
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    // Provide a socket id
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(sockfd));
    // Bind socket to an ip address (gIF_IPV4)
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    // Query port from socket
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(sockfd, _, Pointee(sizeof(sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));
    // Listen on the socket
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd, _)).WillOnce(Return(0));

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    EXPECT_EQ(miniSocket.miniServerSock4, sockfd);
    EXPECT_EQ(miniSocket.miniServerPort4, APPLICATION_LISTENING_PORT);
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
    // No need to close a socket because we haven't got a real socket. It was
    // mocked.
}

TEST_F(StartMiniServerFTestSuite,
       get_miniserver_sockets_with_empty_ip_address) {
    // Binding to an empty text ip address gIF_IPV4 == "" will bind to socket
    // address 0, that is to bind successful to all local ip addresses.
    // TODO: This is also accepted for compatible upnplib code. For native
    // upnplib code this should be a failure. To bind to all local ip addresses
    // gIF_IPV4 should be set to "0.0.0.0".
    gIF_IPV4[0] = '\0'; // Empty ip address
    MINISERVER_REUSEADDR = false;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

    // Due to unmocked bind() it returns successful with a valid ip address
    // instead of failing. Getting a valid ip address is possible because of
    // side effects from previous tests on the global variable gIF_IPV4. It
    // results in an endless loop with this test fixture. So we must have an
    // empty ip address.
    ASSERT_STREQ(gIF_IPV4, "");

    // Test Unit, needs initialized sockets on MS Windows (look at the fixture).
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);

    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);
    // It should return a valid socket (expect not equal invalid).
    EXPECT_NE(miniSocket.miniServerSock4, INVALID_SOCKET);
    // It should return a valid port number.
    EXPECT_EQ(miniSocket.miniServerPort4, APPLICATION_LISTENING_PORT);

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
    EXPECT_EQ(PUPNP_CLOSE_SOCKET(miniSocket.miniServerSock4), 0);
}

TEST_F(StartMiniServerFTestSuite,
       get_miniserver_sockets_with_invalid_ip_address) {
    strcpy(gIF_IPV4, "192.168.129.XXX"); // Invalid ip address
    MINISERVER_REUSEADDR = false;

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

    // Test Unit, needs initialized sockets on MS Windows (look at the fixture).
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);

    // This is OK because we have got socket file descriptors even if we have
    // some wrong ip addresses. Failure is indicated by an INVALID_SOCKET for
    // the particular address_family because it could not be bound to the
    // socket.
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    if (old_code) {
        // This is a bug because we do not have a valid ip address. It is fixed
        // in new_code. The Unit should return INVALID_SOCKET.
        EXPECT_NE(miniSocket.miniServerSock4, INVALID_SOCKET);
        // The Unit should not return a valid port number.
        EXPECT_EQ(miniSocket.miniServerPort4, APPLICATION_LISTENING_PORT);
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " Function should not return a valid socket and port number.\n";

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
        EXPECT_EQ(PUPNP_CLOSE_SOCKET(miniSocket.miniServerSock4), 0);
    else
        EXPECT_EQ(PUPNP_CLOSE_SOCKET(miniSocket.miniServerSock4), -1);
}

TEST(StartMiniServerTestSuite, get_miniserver_sockets_uninitialized) {
    // Test without fixture does not initialize MS Windows sockets. It should
    // never return a valid socket and/or port.

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " Function should fail with win32 uninitialized sockets.\n";
    }

#ifdef _WIN32
    MINISERVER_REUSEADDR = false;
    strcpy(gIF_IPV4, "192.168.200.199");

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);

    if (old_code) {
        // This is a bug and fixed in new_code.
        // std::cout << CYEL "[ BUGFIX   ]" CRES
        // << " Function should fail with win32 uninitialized sockets.\n";
        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);

    } else {

        EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_INIT_FAILED)
            << errStrEx(ret_get_miniserver_sockets, UPNP_E_INIT_FAILED);
    }

    EXPECT_EQ(miniSocket.miniServerSock4, INVALID_SOCKET);
    EXPECT_EQ(miniSocket.miniServerPort4, 0);
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
    EXPECT_EQ(PUPNP_CLOSE_SOCKET(miniSocket.miniServerSock4), -1);
#endif // _WIN32
}

TEST_F(StartMiniServerFTestSuite, get_miniserver_sockets_with_invalid_socket) {
    MINISERVER_REUSEADDR = false;
    strcpy(gIF_IPV4, "192.168.12.9");

    // Initialize needed structure
    MiniServerSockArray miniSocket{};
    NS::InitMiniServerSockArray(&miniSocket);

    // Mock to get an invalid socket id
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(INVALID_SOCKET));

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_sockets =
        NS::get_miniserver_sockets(&miniSocket, 0, 0, 0);
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
}

TEST_F(StartMiniServerFTestSuite, init_socket_suff) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.54.85"};
    struct s_SocketStuff ss4;
    memset(&ss4, 0xAA, sizeof(ss4));

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
    EXPECT_EQ(PUPNP_CLOSE_SOCKET(ss4.fd), 0);
}

TEST_F(StartMiniServerFTestSuite, init_socket_suff_reuseaddr) {
    MINISERVER_REUSEADDR = true;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

    // Test Unit, needs initialized sockets on MS Windows
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(getsockopt(ss4.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen),
              0);
    EXPECT_TRUE(reuseaddr);

    // Important! Otherwise repeated tests will fail later because all file
    // descriptors for the process are consumed.
    EXPECT_EQ(PUPNP_CLOSE_SOCKET(ss4.fd), 0) << std::strerror(errno);
}

TEST_F(StartMiniServerFTestSuite, init_socket_suff_with_invalid_socket) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.99.85"};
    struct s_SocketStuff ss4;

    // Mock to get an invalid socket id
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(SetErrnoAndReturn(EINVAL, SOCKET_ERROR));

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_STREQ(ss4.text_addr, text_addr);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, 16);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "192.168.99.85");
}

TEST_F(StartMiniServerFTestSuite, init_socket_suff_with_invalid_ip_address) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    const char text_addr[]{"192.168.255.256"}; // invalid ip address with .256
    struct s_SocketStuff ss4;

    // Test Unit, needs initialized sockets on MS Windows
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " With an invalid ip address the function call should fail.\n";
        EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);
        EXPECT_STREQ(ss4.text_addr, text_addr);
        EXPECT_GE(ss4.fd, 3);
        EXPECT_EQ(PUPNP_CLOSE_SOCKET(ss4.fd), 0) << std::strerror(errno);

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
    // Set ip address and needed structure. There is no real network adapter on
    // this host with this ip address.
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

TEST(StartMiniServerTestSuite, do_bind_listen_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 600
    // * Mocked bind() returns successful
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns a sockaddr with current ip address and
    //   port

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};
    int sockfd{600};

    struct s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.actual_port = 56789;
    s.serverAddr4->sin_port = htons(s.actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = APPLICATION_LISTENING_PORT;
    s.address_len = sizeof(*s.serverAddr4);

    // If not mocked bind does not know the given ip address and fails.
    // The Unit will loop through all port numbers to find a free port
    // but will never find one. The program hungs in an endless loop.

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr_set(text_addr, APPLICATION_LISTENING_PORT);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(sockfd, _, Pointee(sizeof(sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT + 1);
    EXPECT_EQ(s.actual_port, APPLICATION_LISTENING_PORT);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST_F(StartMiniServerFTestSuite, do_bind_listen_with_wrong_socket) {
    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"0.0.0.0"};

    struct s_SocketStuff s;
    EXPECT_EQ(init_socket_suff(&s, text_addr, 4), 0);
    EXPECT_EQ(PUPNP_CLOSE_SOCKET(s.fd), 0) << std::strerror(errno);
    // The socket id wasn't got from a socket() call now and should trigger an
    // error.
    s.fd = 32000;
    s.try_port = 65534;

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SOCKET_BIND);
}

TEST_F(StartMiniServerFTestSuite, do_bind_listen_with_failed_listen) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 600
    // * Mocked bind() returns successful
    // * Mocked listen() returns error

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};
    const int actual_port{0};
    const int sockfd{600};

    struct s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.actual_port = actual_port;
    s.serverAddr4->sin_port = htons(s.actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = APPLICATION_LISTENING_PORT;
    s.address_len = sizeof(*s.serverAddr4);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, bind(s.fd, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(s.fd, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EOPNOTSUPP, -1));

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_LISTEN)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_LISTEN);

    // sock_close() is not needed because there is no socket called.
}

TEST(StartMiniServerTestSuite, do_bind_listen_address_in_use) {
    // Configure expected system calls:
    // * Use fictive socket file descriptors 600 and 601 with actual port 52534
    // * Mocked bind() returns successful
    // * Mocked listen() returns error with errno EADDRINUSE
    // * Mocked getsockname() returns a sockaddr with current ip address and
    //   port

    if (old_code)
        GTEST_SKIP() << CYEL "[ BUGFIX   ]" CRES
                     << " Unit should not loop through about 50000 ports to "
                        "find one free port.";

    MINISERVER_REUSEADDR = false;
    const char text_addr[]{"192.168.54.188"};
    const int actual_port{52534};
    const int sockfd_inuse{600};
    const int sockfd_free{601};

    struct s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.actual_port = actual_port;
    s.serverAddr4->sin_port = htons(s.actual_port);
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd_inuse;
    s.try_port = actual_port + 1;
    s.address_len = sizeof(*s.serverAddr4);

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr_set(text_addr, actual_port + 1);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    // A successful bind is expected but listen should fail with "address in
    // use"
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd_inuse, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd_inuse, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, SOCKET_ERROR));
    // A second attempt will call init_socket_suff() to get a new socket.
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(sockfd_free));
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd_free, _, _)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd_free, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(sockfd_free, _, Pointee(sizeof(sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Test Unit
    int ret_get_do_bind_listen = do_bind_listen(&s);
    EXPECT_EQ(ret_get_do_bind_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_get_do_bind_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), actual_port + 1);
    EXPECT_EQ(s.fd, sockfd_free);
    EXPECT_EQ(s.try_port, actual_port + 2);
    EXPECT_EQ(s.actual_port, actual_port + 1);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST_F(DoBindFTestSuite, bind_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 511
    // * Actual used port is 56789
    // * Next port to try is 56790
    // * Mocked bind() returns successful

    // Provide needed data for the Unit
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    uint16_t actual_port{56789};
    uint16_t try_port{56790};

    struct s_SocketStuff s;
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
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));

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
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.try_port, try_port + 1);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, actual_port);

    } else {

        EXPECT_EQ(s.actual_port, actual_port + 1);
    }

    // sock_close() is not needed because there is no socket called.
}

TEST_F(DoBindFTestSuite, bind_with_invalid_argument) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 512
    // * Actual used port is 56890
    // * Next port to try is 56891
    // * Mocked bind() returns EINVAL

    // Provide needed data for the Unit
    const int sockfd{512};
    const char text_addr[]{"192.168.202.233"};
    uint16_t actual_port{56890};
    uint16_t try_port{56891};

    struct s_SocketStuff s;
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
    class Mock_sys_socket mocked_sys_socketObj;
#ifdef _WIN32
    WSASetLastError(WSAEINVAL);
#endif

    if (old_code) {
        // If bind() always returns failure due to unchanged invalid argument
        // the Unit will hung in an endless loop. There is no exit for this
        // condition. Here it will only stop after three loops because bind()
        // returns successful at last. This is fixed in new code.
        EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
            .WillOnce(SetErrnoAndReturn(EINVAL, -1))
            .WillOnce(SetErrnoAndReturn(EINVAL, -1))
            .WillOnce(Return(0));

        // Test Unit
        // This wrong condition is expected if the code hasn't changed.
        int ret_do_bind = do_bind(&s);
        EXPECT_EQ(ret_do_bind, UPNP_E_SUCCESS)
            << errStrEx(ret_do_bind, UPNP_E_SUCCESS);

    } else {

        EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
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
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));

    if (old_code) {
        // See notes above about the endless loop. Expected values here are
        // meaningless and only tested to watch code changes.
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " do_bind() should never hung with testing all free ports "
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

TEST(DoBindTestSuite, bind_with_try_port_overrun) {
    // This setup will 'try_port' overrun after 65535 to 0. The overrun should
    // finish the search for a free port to bind.
    //
    // Configure expected system calls:
    // * Use fictive socket file descriptor 511
    // * Actual used port is 56789
    // * Next port to try is 65533
    // * Mocked bind() returns always failure with errno EINVAL

    // Provide needed data for the Unit
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};

    struct s_SocketStuff s;
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

    // Mock socket
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    // Mock system function, must also set errno
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
        .Times(3)
        .WillRepeatedly(SetErrnoAndReturn(EADDRINUSE, -1));

    // Test Unit
    int ret_do_bind = do_bind((s_SocketStuff*)&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 65535);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Next try to bind a port should start with "
                     "APPLICATION_LISTENING_PORT but not with port 0.\n";
        EXPECT_EQ(s.try_port, 0);
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, 56789);

    } else {

        EXPECT_EQ(s.try_port, APPLICATION_LISTENING_PORT);
        EXPECT_EQ(s.actual_port, 65535);
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
    struct s_SocketStuff s;

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
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        .WillOnce(SetErrnoAndReturn(EADDRINUSE, -1))
        // The system library never reset errno so don't do it here.
        // .WillOnce(SetErrnoAndReturn(0, 0));
        .WillOnce(Return(0));

    // Test Unit
    int ret_do_bind = do_bind((s_SocketStuff*)&s);

    if (old_code) {
        EXPECT_EQ(s.try_port, 0);

    } else {

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
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " The actual_port number should be set to the new number.\n";
        EXPECT_EQ(s.actual_port, 56789);

    } else {

        EXPECT_EQ(s.actual_port, 65535);
    }
}

TEST(DoBindTestSuite, bind_with_empty_parameter) {
    // With this test we have an initialized ip_version = 0, instead of valid 4
    // or 6. Switching for this value will never find an end.
    if (old_code)
        GTEST_SKIP() << CYEL "[ BUGFIX   ]" CRES
                     << " This test stuck the program in an endless loop.";

    struct s_SocketStuff s {};
    s.serverAddr = (struct sockaddr*)&s.ss;

    // Test Unit
    int ret_do_bind;
    ret_do_bind = do_bind((s_SocketStuff*)&s);

    EXPECT_EQ(ret_do_bind, UPNP_E_INVALID_PARAM)
        << errStrEx(ret_do_bind, UPNP_E_INVALID_PARAM);
}

TEST_F(DoBindFTestSuite, bind_with_wrong_ip_version_assignment) {
    // Setting ip_version = 6 and sin_family = AF_INET and vise versa does not
    // fit. Provide needed data for the Unit.
    const int sockfd{511};
    const char text_addr[]{"192.168.101.233"};
    uint16_t try_port{65533};

    struct s_SocketStuff s;
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
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " This should not loop through all free port numbers. It "
                     "will always fail.\n";
    }

    // Test Unit
    int ret_do_bind = do_bind((s_SocketStuff*)&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);

    // Set ip_version = 4 and sin_family = AF_INET6
    s.ip_version = 4;
    s.serverAddr4->sin_family = AF_INET6;
    s.try_port = try_port;

    // Test Unit
    ret_do_bind = do_bind((s_SocketStuff*)&s);
    EXPECT_EQ(ret_do_bind, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_do_bind, UPNP_E_SOCKET_BIND);
}

TEST(StartMiniServerTestSuite, do_listen_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 512
    // * Actual used port 60000 will be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns successful

    // Provide needed data for the Unit
    const int sockfd{512};
    const char text_addr[] = "192.168.202.233";
    uint16_t actual_port{60000};
    uint16_t try_port{0}; // not used

    struct s_SocketStuff s;
    // Fill all fields of struct s_SocketStuff
    s.serverAddr = (struct sockaddr*)&s.ss;
    s.ip_version = 4;
    s.text_addr = text_addr;
    s.serverAddr4->sin_family = AF_INET;
    s.serverAddr4->sin_port = 0; // not used
    inet_pton(AF_INET, text_addr, &s.serverAddr4->sin_addr);
    s.fd = sockfd;
    s.try_port = try_port; // not used
    s.actual_port = 0;
    s.address_len = sizeof(*s.serverAddr4);

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr_set(text_addr, actual_port);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(sockfd, _, Pointee(sizeof(sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_SUCCESS)
        << errStrEx(ret_do_listen, UPNP_E_SUCCESS);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, actual_port);
    EXPECT_EQ(s.try_port, try_port); // not used
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(StartMiniServerTestSuite, do_listen_not_supported) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 612
    // * Actual used port will not be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns with EOPNOTSUPP
    // * Mocked getsockname() is not called

    // Provide needed data for the Unit
    const int sockfd{512};
    const char text_addr[] = "192.168.101.203";

    struct s_SocketStuff s;
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
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd, SOMAXCONN))
        .WillOnce(SetErrnoAndReturn(EOPNOTSUPP, -1));
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _)).Times(0);

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_LISTEN)
        << errStrEx(ret_do_listen, UPNP_E_LISTEN);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 0);
    EXPECT_EQ(s.try_port, 0); // not used
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(StartMiniServerTestSuite, do_listen_insufficient_resources) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 512
    // * Actual used port will not be set
    // * Next port to try is 0 because not used here
    // * Mocked listen() returns successful
    // * Mocked getsockname() returns with ENOBUFS

    // Provide needed data for the Unit
    const int sockfd{512};
    const char text_addr[] = "192.168.101.203";

    struct s_SocketStuff s;
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
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_STREAM, 0)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, listen(sockfd, SOMAXCONN)).Times(1);
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, -1));

    // Test Unit
    int ret_do_listen = do_listen(&s);
    EXPECT_EQ(ret_do_listen, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_do_listen, UPNP_E_INTERNAL_ERROR);

    // Check all fields of struct s_SocketStuff
    EXPECT_EQ(s.ip_version, 4);
    EXPECT_STREQ(s.text_addr, text_addr);
    EXPECT_EQ(s.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ntohs(s.serverAddr4->sin_port), 0); // not used
    EXPECT_STREQ(inet_ntoa(s.serverAddr4->sin_addr), text_addr);
    EXPECT_EQ(s.fd, sockfd);
    EXPECT_EQ(s.actual_port, 0);
    EXPECT_EQ(s.try_port, 0); // not used
    EXPECT_EQ(s.address_len, sizeof(*s.serverAddr4));
}

TEST(StartMiniServerTestSuite, get_port_successful) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 1000
    // * Actual socket used port is 55555
    // * Mocked getsockname() returns successful

    // Provide needed data for the Unit
    const int sockfd{1000};
    const char text_addr[] = "192.168.154.188";
    uint16_t actual_port{55555};
    // This is for the returned port number
    uint16_t port{0xAAAA};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr_set(text_addr, actual_port);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Test Unit
    EXPECT_EQ(get_port(sockfd, &port), 0);

    EXPECT_EQ(port, actual_port);
}

TEST(StartMiniServerTestSuite, get_port_fails) {
    // Configure expected system calls:
    // * Use fictive socket file descriptor 900
    // * Mocked getsockname() fails with insufficient resources (ENOBUFS).

    // Provide needed data for the Unit
    const int sockfd{900};
    // This is for the returned port number
    uint16_t port{0xAAAA};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname(). It will be empty.
    struct SockAddr sock;

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr),
                        SetErrnoAndReturn(ENOBUFS, -1)));

    // Test Unit
    EXPECT_EQ(get_port(sockfd, &port), -1);

    EXPECT_EQ(errno, ENOBUFS);
    EXPECT_EQ(port, 0xAAAA);
}

TEST_F(StartMiniServerFTestSuite, get_miniserver_stopsock) {
    // Here we test a real connection to the loopback device. This needs
    // initialization of sockets on MS Windows which is done with the fixture.
    // We also have to close the socket.
    MiniServerSockArray out;
    NS::InitMiniServerSockArray(&out);

    // Test Unit, needs initialized sockets on MS Windows
    int ret_get_miniserver_stopsock = NS::get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SUCCESS);

    EXPECT_NE(out.miniServerStopSock, 0);
    EXPECT_NE(out.stopPort, 0);
    EXPECT_EQ(out.stopPort, miniStopSockPort);

    // Provide a sockaddr structure to getsockname().
    struct SockAddr sock;
    socklen_t sslen = sizeof(sock.addr_ss);

    // Get address information direct from the bound socket
    ASSERT_EQ(getsockname(out.miniServerStopSock, sock.addr, &sslen), 0);
    // and verify its settings
    EXPECT_EQ(sock.addr_in->sin_family, AF_INET);
    EXPECT_EQ(sock.addr_get_port(), miniStopSockPort);
    EXPECT_EQ(sock.addr_get(), "127.0.0.1");

    // Close socket
    EXPECT_EQ(sock_close(out.miniServerStopSock), 0);
}

TEST(StartMiniServerTestSuite, get_miniserver_stopsock_fails) {
    // Configure expected system calls:
    // * Get a socket() fails with EACCES (Permission denied).
    // * bind() is not called.
    // * getsockname() is not called.

    // Provide needed data for the Unit
    MiniServerSockArray out;
    NS::InitMiniServerSockArray(&out);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(SetErrnoAndReturn(EACCES, -1));
    EXPECT_CALL(mocked_sys_socketObj, bind(_, _, _)).Times(0);
    EXPECT_CALL(mocked_sys_socketObj, getsockname(_, _, _)).Times(0);

    // Test Unit
    int ret_get_miniserver_stopsock = NS::get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_OUTOF_SOCKET)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_OUTOF_SOCKET);

    // Close socket; we don't need to close a mocked socket
}

TEST(StartMiniServerTestSuite, get_miniserver_stopsock_bind_fails) {
    // Configure expected system calls:
    // * socket() returns file descriptor 890.
    // * bind() fails with ENOMEM.
    // * getsockname() is not called.

    // Provide needed data for the Unit
    MiniServerSockArray out;
    NS::InitMiniServerSockArray(&out);
    const int sockfd{890};

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(sockfd));
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOMEM, -1));
    EXPECT_CALL(mocked_sys_socketObj, getsockname(_, _, _)).Times(0);

    // Test Unit
    int ret_get_miniserver_stopsock = NS::get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_SOCKET_BIND);

    // Close socket; we don't need to close a mocked socket
}

TEST(StartMiniServerTestSuite, get_miniserver_stopsock_getsockname_fails) {
    // Configure expected system calls:
    // * socket() returns file descriptor 888.
    // * bind() returns successful.
    // * getsockname() fails with ENOBUFS (Cannot allocate memory).

    // Provide needed data for the Unit
    MiniServerSockArray out;
    NS::InitMiniServerSockArray(&out);
    const int sockfd{888};

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, socket(AF_INET, SOCK_DGRAM, 0))
        .WillOnce(Return(sockfd));
    EXPECT_CALL(mocked_sys_socketObj, bind(sockfd, _, _)).WillOnce(Return(0));
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, -1));

    // Test Unit
    int ret_get_miniserver_stopsock = NS::get_miniserver_stopsock(&out);
    EXPECT_EQ(ret_get_miniserver_stopsock, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_get_miniserver_stopsock, UPNP_E_INTERNAL_ERROR);

    // Close socket; we don't need to close a mocked socket
}

TEST(RunMiniServerTestSuite, receive_from_stopSock) {
    const int sockfd{401};
    struct SockAddr sock;
    sock.addr_set("192.168.167.166", 54321);

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(
        mocked_sys_socketObj,
        recvfrom(sockfd, _, 25, 0, _, Pointee(sizeof(sockaddr_storage))))
        .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"),
                        SetArgPointee<4>(*sock.addr), Return(8)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    EXPECT_EQ(NS::receive_from_stopSock(sockfd, &rdSet), 1);
}

TEST(RunMiniServerTestSuite, receive_from_stopSock_not_selected) {
    const int sockfd{402};

    fd_set rdSet;
    FD_ZERO(&rdSet);
    // Socket not selected to be received
    // FD_SET(sockfd, &rdSet);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, recvfrom(_, _, _, _, _, _)).Times(0);

    // Test Unit
    EXPECT_EQ(NS::receive_from_stopSock(sockfd, &rdSet), 0);
}

TEST(RunMiniServerTestSuite, receive_from_stopSock_no_bytes) {
    const int sockfd{403};
    struct SockAddr sock;
    sock.addr_set("192.168.167.168", 54323);

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(
        mocked_sys_socketObj,
        recvfrom(sockfd, _, 25, 0, _, Pointee(sizeof(sockaddr_storage))))
        .WillOnce(
            DoAll(StrCpyToArg<1>(""), SetArgPointee<4>(*sock.addr), Return(0)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    EXPECT_EQ(NS::receive_from_stopSock(sockfd, &rdSet), 0);
}

TEST(RunMiniServerTestSuite, receive_from_stopSock_nothing_todo) {
    SOCKET sockfd{404};
    struct SockAddr sock;
    sock.addr_set("192.168.167.169", 54324);

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(
        mocked_sys_socketObj,
        recvfrom(sockfd, _, 25, 0, _, Pointee(sizeof(sockaddr_storage))))
        .WillOnce(DoAll(StrCpyToArg<1>("NothingToDo"),
                        SetArgPointee<4>(*sock.addr), Return(11)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    if (old_code)
        EXPECT_EQ(NS::receive_from_stopSock(sockfd, &rdSet), 0);
    else
        EXPECT_EQ(NS::receive_from_stopSock(sockfd, &rdSet), 1);
}

TEST(RunMiniServerTestSuite, RunMiniServer) {
    // This would start some other threads. We run into dynamic problems with
    // parallel running threads here. For example running the miniserver with
    // schedule_request_job() in a new thread cannot be finished before the
    // mocked miniserver shutdown in the calling thread has been executed at
    // Unit end. This is why I prevent starting other threads. We only test
    // initialize running the miniserver and stopping it.

    // Initialize the threadpool. Don't forget to shutdown the threadpool at the
    // end. nullptr means to use default attributes.
    EXPECT_EQ(ThreadPoolInit(&gMiniServerThreadPool, nullptr), 0);
    // Prevent to add jobs, we test jobs isolated.
    gMiniServerThreadPool.shutdown = 1;
    // EXPECT_EQ(TPAttrSetMaxJobsTotal(&gMiniServerThreadPool.attr, 0), 0);

    // Initialize needed data
    SOCKET listen_sockfd = 201;
    uint16_t listen_port = 301;
    SOCKET connected_sockfd = 202;
    uint16_t connected_port = 302;
    SOCKET stop_sockfd = 203;
    uint16_t stop_port = 303;
    int select_nfds = stop_sockfd + 1; // See man select

    MiniServerSockArray* minisock =
        (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
    ASSERT_NE(minisock, nullptr);
    NS::InitMiniServerSockArray(minisock);
    minisock->miniServerSock4 = listen_sockfd;
    minisock->miniServerStopSock = stop_sockfd;
    minisock->stopPort = stop_port;
    minisock->miniServerPort4 = listen_port;

    { // Scope of mocking only within this block

        // Mock functions from standard system library
        class Mock_sys_select mocked_sys_selectObj;

        if (old_code) {
            std::cout << CYEL "[ BUGFIX   ]" CRES
                      << " Max socket fd for select() not setting to 0 if "
                         "INVALID_SOCKET in MiniServerSockArray on WIN32.\n";
#ifdef _WIN32
            EXPECT_CALL(mocked_sys_selectObj, select(0, _, nullptr, _, nullptr))
                .WillOnce(Return(1));
#else
            EXPECT_CALL(mocked_sys_selectObj,
                        select(select_nfds, _, nullptr, _, nullptr))
                .WillOnce(Return(1));
#endif
        } else {

            EXPECT_CALL(mocked_sys_selectObj,
                        select(select_nfds, _, nullptr, _, nullptr))
                .WillOnce(Return(1));
        }

        class Mock_sys_socket mocked_sys_socketObj;
        struct SockAddr connected_sock;
        connected_sock.addr_set("192.168.200.201", connected_port);
        EXPECT_CALL(
            mocked_sys_socketObj,
            accept(listen_sockfd, NotNull(), Pointee(sizeof(sockaddr_storage))))
            .WillOnce(DoAll(SetArgPointee<1>(*connected_sock.addr),
                            Return(connected_sockfd)));

        struct SockAddr localhost_sock;
        localhost_sock.addr_set("127.0.0.1", stop_port);

        if (old_code) {
            std::cout << CYEL "[ BUGFIX   ]" CRES
                      << " Unit must not endless loop with wrong \"shutdown\" "
                         "message instead of \"ShutDown\".\n";
            EXPECT_CALL(mocked_sys_socketObj,
                        recvfrom(stop_sockfd, _, 25, 0, _,
                                 Pointee(sizeof(sockaddr_storage))))
                .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"),
                                SetArgPointee<4>(*localhost_sock.addr),
                                Return(8)));

        } else {

            EXPECT_CALL(mocked_sys_socketObj,
                        recvfrom(stop_sockfd, _, 25, 0, _,
                                 Pointee(sizeof(sockaddr_storage))))
                .WillOnce(DoAll(StrCpyToArg<1>("shutdown"),
                                SetArgPointee<4>(*localhost_sock.addr),
                                Return(8)));

            EXPECT_CALL(mocked_sys_socketObj,
                        getsockopt(listen_sockfd, SOL_SOCKET, SO_ERROR, _, _))
                .WillOnce(Return(0));
            EXPECT_CALL(mocked_sys_socketObj,
                        getsockopt(INVALID_SOCKET, SOL_SOCKET, SO_ERROR, _, _))
                .Times(7)
                .WillRepeatedly(SetErrnoAndReturn(EBADF, -1));
        }

        std::cout << CYEL "[ BUG      ]" CRES
                  << " Unit must not expect its argument MiniServerSockArray* "
                     "to be on the heap and free it.\n";

        // Test Unit
        NS::RunMiniServer(minisock);

    } // End scope of mocking, objects within the block will be destructed.

    // Shutdown the threadpool.
    EXPECT_EQ(ThreadPoolShutdown(&gMiniServerThreadPool), 0);
}

TEST(RunMiniServerTestSuite, ssdp_read) {
    SOCKET ssdp_sockfd = 208;
    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(ssdp_sockfd, &rdSet);

    // Test Unit
    NS::ssdp_read(ssdp_sockfd, &rdSet);
}

TEST(RunMiniServerTestSuite, web_server_accept) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET listen_sockfd = 205;
    SOCKET connected_sockfd = 206;
    uint16_t connected_port = 306;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(listen_sockfd, &set);

    // Initialize the threadpool. Don't forget to shutdown the threadpool at the
    // end. nullptr means to use default attributes.
    EXPECT_EQ(ThreadPoolInit(&gMiniServerThreadPool, nullptr), 0);
    // Prevent to add jobs, we test jobs isolated. See note at
    // TEST(RunMiniServerTestSuite, RunMiniServer).
    // gMiniServerThreadPool.shutdown = 1;
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&gMiniServerThreadPool.attr, 0), 0);

    { // Scope of mocking only within this block

        class Mock_sys_socket mocked_sys_socketObj;
        struct SockAddr connected_sock;
        connected_sock.addr_set("192.168.201.202", connected_port);
        EXPECT_CALL(
            mocked_sys_socketObj,
            accept(listen_sockfd, NotNull(), Pointee(sizeof(sockaddr_storage))))
            .WillOnce(DoAll(SetArgPointee<1>(*connected_sock.addr),
                            Return(connected_sockfd)));

        // Capture output to stderr
        class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
        captureObj.start();

        // Test Unit
        NS::web_server_accept(listen_sockfd, &set);

        // Get captured output
        std::string capturedStderr = captureObj.get();
#ifdef DEBUG
        if (old_code)
            EXPECT_THAT(capturedStderr,
                        ContainsStdRegex(" UPNP-MSER-2: .* mserv 206: cannot "
                                         "schedule request\n"));
        else
            EXPECT_THAT(
                capturedStderr,
                ContainsStdRegex(
                    " connected to host 192.168.201.202:306 with socket "
                    "206\n.*\n.*\n.* "
                    "UPNP-MSER-1: .* mserv 206: cannot schedule request\n"));
#else
        if (old_code)
            EXPECT_THAT(
                capturedStderr,
                ContainsStdRegex("libupnp ThreadPoolAdd too many jobs: 0\n"));
        else
            EXPECT_THAT(
                capturedStderr,
                ContainsStdRegex("libupnp ThreadPoolAdd too many jobs: 0\n"));
#endif
    } // End scope of mocking, objects within the block will be destructed.

    // Shutdown the threadpool.
    EXPECT_EQ(ThreadPoolShutdown(&gMiniServerThreadPool), 0);
}

TEST(RunMiniServerTestSuite, web_server_accept_with_invalid_socket) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET listen_sockfd = INVALID_SOCKET;

    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, accept(_, _, _)).Times(0);

    // Capture output to stderr
    class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    NS::web_server_accept(listen_sockfd, nullptr);

    // Get captured output
    std::string capturedStderr = captureObj.get();
    if (old_code)
        EXPECT_TRUE(capturedStderr.empty());
    else
#ifdef DEBUG
        EXPECT_THAT(capturedStderr,
                    ContainsStdRegex(" UPNP-MSER-1: .* invalid socket\\(-1\\) "
                                     "or set\\(.*\\)\\.\n"));
#else
        EXPECT_TRUE(capturedStderr.empty());
#endif
}

TEST(RunMiniServerTestSuite, web_server_accept_with_empty_set) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET listen_sockfd = 207;
    fd_set set;
    FD_ZERO(&set);

    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, accept(_, _, _)).Times(0);

    // Capture output to stderr
    class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    NS::web_server_accept(listen_sockfd, &set);

    // Get captured output
    std::string capturedStderr = captureObj.get();
    if (old_code)
        EXPECT_TRUE(capturedStderr.empty());
    else
#ifdef DEBUG
        EXPECT_THAT(capturedStderr,
                    ContainsStdRegex(" UPNP-MSER-1: .* invalid socket\\(207\\) "
                                     "or set\\(.*\\)\\.\n"));
#else
        EXPECT_TRUE(capturedStderr.empty());
#endif
}

TEST_F(RunMiniServerFTestSuite, fdset_if_valid) {
    fd_set rdSet;
    FD_ZERO(&rdSet);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sockfd, INVALID_SOCKET) << std::strerror(errno);

    // Test Unit
    NS::fdset_if_valid(sockfd, &rdSet);

    EXPECT_NE(FD_ISSET(sockfd, &rdSet), 0);

    EXPECT_EQ(PUPNP_CLOSE_SOCKET(sockfd), 0) << std::strerror(errno);
}

TEST_F(RunMiniServerFTestSuite, fdset_if_valid_with_closed_socket) {
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " Due to undefined behavior of FD_SET with invalid fd "
               "this randomly terminated with 'stack smashing detected'.\n";
    } else {

#ifndef DEBUG
        GTEST_SKIP()
            << "             This test only runs with build type DEBUG.";
#else
        class CLogging loggingObj; // Output only with build type DEBUG.

        constexpr SOCKET sockfd{1111};
        fd_set rdSet;
        FD_ZERO(&rdSet);

        // Capture output to stderr
        class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
        captureObj.start();

        // Test Unit
        NS::fdset_if_valid(sockfd, &rdSet);

        // We cannot verify with FD_ISSET due to undefined behavior with invalid
        // socket.
        // EXPECT_EQ(FD_ISSET(sockfd, &rdSet), 0);

        // Get captured output
        std::string capturedStderr = captureObj.get();
        EXPECT_THAT(capturedStderr,
                    ContainsStdRegex(" UPNP-MSER-2: .* FD_SET for select\\(\\) "
                                     "failed with socket 1111."));
#endif
    }
}

TEST(RunMiniServerTestSuite, schedule_request_job) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET connected_sockfd = 202;
    uint16_t connected_port = 302;
    struct SockAddr sock;
    sock.addr_set("192.168.1.1", connected_port);

    // Initialize the threadpool. Don't forget to shutdown the threadpool at the
    // end. nullptr means to use default attributes.
    EXPECT_EQ(ThreadPoolInit(&gMiniServerThreadPool, nullptr), 0);
    // Prevent to add jobs, we test jobs isolated. See note at
    // TEST(RunMiniServerTestSuite, RunMiniServer).
    // gMiniServerThreadPool.shutdown = 1;
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&gMiniServerThreadPool.attr, 0), 0);

    // Capture output to stderr
    class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    NS::schedule_request_job(connected_sockfd, sock.addr);

    // Get captured output
    std::string capturedStderr = captureObj.get();
#ifdef DEBUG
    EXPECT_THAT(
        capturedStderr,
        ContainsStdRegex(" UPNP-MSER-\\d: .* 202: cannot schedule request\n"));
#else
    EXPECT_THAT(capturedStderr,
                ContainsStdRegex("libupnp ThreadPoolAdd too many jobs: 0\n"));
#endif
    // Shutdown the threadpool.
    EXPECT_EQ(ThreadPoolShutdown(&gMiniServerThreadPool), 0);
}

TEST(RunMiniServerTestSuite, handle_request) {
    GTEST_SKIP()
        << "Still needs to be done when I have understood http_RecvMessage().";
}

TEST(RunMiniServerTestSuite, handle_request_with_invalid_socket) {
    GTEST_SKIP()
        << "Still needs to be done when I have understood http_RecvMessage().";

    class CLogging loggingObj; // Output only with build type DEBUG.

    struct mserv_request_t request {};
    request.connfd = 999;

    // Test Unit
    handle_request(&request);
}

TEST_F(RunMiniServerFTestSuite, free_handle_request_arg) {
    // Provide request structure
    struct mserv_request_t* request =
        (struct mserv_request_t*)malloc(sizeof(*request));
    memset(request, 0, sizeof *request);
    // and set a socket
    request->connfd = socket(AF_INET, SOCK_STREAM, 0);

    // Test Unit
    free_handle_request_arg(request);
}

TEST_F(RunMiniServerFTestSuite, free_handle_request_arg_with_invalid_socket) {
    // Provide request structure
    struct mserv_request_t* request =
        (struct mserv_request_t*)malloc(sizeof(*request));
    memset(request, 0, sizeof *request);
    // and set an invalid socket
    request->connfd = INVALID_SOCKET;

    // Test Unit
    free_handle_request_arg(request);
}

TEST(RunMiniServerDeathTest, free_handle_request_arg_with_nullptr_to_struct) {
    // Provide request structure
    struct mserv_request_t* request{nullptr};

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " free_handle_re4quest with nullptr must not segfault.\n";
        EXPECT_DEATH(free_handle_request_arg(request), "");

    } else {

        free_handle_request_arg(request);
    }
}

TEST(RunMiniServerTestSuite, handle_error) {
    GTEST_SKIP() << "Still needs to be done when I have made the test to "
                    "http_SendStatusResponse.";
}

TEST(RunMiniServerTestSuite, dispatch_request) {
    GTEST_SKIP() << "Still needs to be done when we have complete tests for "
                    "httpreadwrite.";
}

TEST(RunMiniServerTestSuite, getNumericHostRedirection) {
    // getNumericHostRedirection() returns the ip address with port as text
    // (e.g. "192.168.1.2:54321") that is bound to a socket.

    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET sockfd{405};
    char host_port[INET6_ADDRSTRLEN + 1 + 5]{"<no message>"};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr_set("192.168.123.122", 54321);

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Test Unit
    EXPECT_TRUE(
        getNumericHostRedirection(sockfd, host_port, sizeof(host_port)));

    EXPECT_STREQ(host_port, "192.168.123.122:54321");
}

TEST(RunMiniServerTestSuite,
     getNumericHostRedirection_with_insufficient_resources) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET sockfd{406};
    char host_port[INET6_ADDRSTRLEN + 1 + 5]{"<no message>"};

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, -1));

    // Capture output to stderr
    class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    EXPECT_FALSE(
        getNumericHostRedirection(sockfd, host_port, sizeof(host_port)));

    // Get captured output
    std::string capturedStderr = captureObj.get();
    if (old_code)
        EXPECT_TRUE(capturedStderr.empty());
    else
#ifdef DEBUG
        EXPECT_THAT(
            capturedStderr,
            ContainsStdRegex(
                " UPNP-MSER-1: .*Error with systemcall getsockname\\(\\)\\. "
                "[Nn]o buffer space"));
#else
        EXPECT_TRUE(capturedStderr.empty());
#endif
    EXPECT_STREQ(host_port, "<no message>");
}

TEST(RunMiniServerTestSuite,
     getNumericHostRedirection_with_wrong_address_family) {
    class CLogging loggingObj; // Output only with build type DEBUG.

    SOCKET sockfd{407};
    char host_port[INET6_ADDRSTRLEN + 1 + 5]{"<no message>"};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr sock;
    sock.addr->sa_family = AF_UNIX;

    // Mock system functions
    class Mock_sys_socket mocked_sys_socketObj;
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(*sock.addr), Return(0)));

    // Capture output to stderr
    class CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    bool ret_getNumericHostRedirection =
        getNumericHostRedirection(sockfd, host_port, sizeof(host_port));

    // Get captured output
    std::string capturedStderr = captureObj.get();

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " A wrong but accepted address family AF_UNIX should "
                     "return an error.\n";
        EXPECT_TRUE(ret_getNumericHostRedirection); // wrong
        EXPECT_TRUE(capturedStderr.empty());
        EXPECT_STREQ(host_port, "0.0.0.0:0"); // wrong

    } else {

        EXPECT_FALSE(ret_getNumericHostRedirection);
#ifdef DEBUG
        EXPECT_THAT(
            capturedStderr,
            ContainsStdRegex(" UPNP-MSER-1: .*Invalid address family 1\\.\n"));
#else
        EXPECT_TRUE(capturedStderr.empty());
#endif
        EXPECT_STREQ(host_port, "<no message>");
    }
}

TEST(RunMiniServerTestSuite, host_header_is_numeric) {
    char host_port[INET_ADDRSTRLEN + 1 + 5]{"192.168.88.99:59876"};

    EXPECT_TRUE(host_header_is_numeric(host_port, sizeof(host_port)));
}

TEST(RunMiniServerTestSuite, host_header_is_numeric_with_invalid_ip_address) {
    char host_port[INET_ADDRSTRLEN + 1 + 5]{"192.168.88.256:59877"};

    EXPECT_FALSE(host_header_is_numeric(host_port, sizeof(host_port)));
}

TEST(RunMiniServerTestSuite, host_header_is_numeric_with_empty_port) {
    char host_port[INET_ADDRSTRLEN + 1 + 5]{"192.168.88.99:"};

    EXPECT_TRUE(host_header_is_numeric(host_port, sizeof(host_port)));
}

TEST(RunMiniServerTestSuite, host_header_is_numeric_without_port) {
    char host_port[INET_ADDRSTRLEN + 1 + 5]{"192.168.88.99"};

    std::cout << CYEL "[ BUG      ]" CRES
              << " Because host_header with an empty port is valid then "
                 "without port it should also be valid.\n";
    // wrong
    EXPECT_FALSE(host_header_is_numeric(host_port, sizeof(host_port)));
}

TEST_F(StopMiniServerFTestSuite, sock_close) {
    // Close invalid sockets
    EXPECT_EQ(sock_close(INVALID_SOCKET), -1);
    EXPECT_EQ(sock_close(1234), -1);

    // Get a valid socket, needs initialized sockets on MS Windows with fixture.
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sockfd, -1);
    // Close a valid socket.
    EXPECT_EQ(sock_close(sockfd), 0);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    // class CLogging loggingObj; // Output only with build type DEBUG.
#include "upnplib/gtest_main.inc"
}
