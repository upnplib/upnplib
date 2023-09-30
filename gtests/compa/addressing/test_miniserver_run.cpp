// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-09-30

// All functions of the miniserver module have been covered by a gtest. Some
// tests are skipped and must be completed when missed information is
// available.

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include <pupnp/upnp/src/genlib/miniserver/miniserver.cpp>
#else
#include <compa/src/genlib/miniserver/miniserver.cpp>
#endif

#include <upnplib/gtest.hpp>
#include <upnplib/sockaddr.hpp>

#include <umock/sys_socket_mock.hpp>


namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrictMock;

using ::upnplib::SSockaddr_storage;

using ::upnplib::testing::StrCpyToArg;


// Miniserver Run TestSuite
// ========================
class MiniServerRunFTestSuite : public ::testing::Test {
  protected:
    // clang-format off
    // Instantiate mocking objects.
    StrictMock<umock::Sys_socketMock> m_sys_socketObj;
    // Inject the mocking objects into the tested code.
    umock::Sys_socket sys_socket_injectObj = umock::Sys_socket(&m_sys_socketObj);
    // clang-format on
};

TEST_F(MiniServerRunFTestSuite, RunMiniServer_successful) {
    // IMPORTANT! There is a limit FD_SETSIZE = 1024 for socket file
    // descriptors that can be used with 'select()'. This limit is not given
    // when using 'poll()' or 'epoll' instead. Old_code uses 'select()' so we
    // must test with this limited socket file descriptors. Otherwise we may
    // get segfaults with 'FD_SET()'. For details have a look at 'man select'.
    //
    // This would start some other threads. We run into dynamic problems with
    // parallel running threads here. For example running the miniserver with
    // schedule_request_job() in a new thread cannot be finished before the
    // mocked miniserver shutdown in the calling thread has been executed at
    // Unit end. This is why I prevent starting other threads. We only test
    // initialize running the miniserver and stopping it.
    //
    // We have 7 socket file descriptors and additional 2 with client APIs,
    // that are used to listen to the different IPv4 and IPv6 protocols for the
    // miniserver (4 fds), the ssdp service (3 fds) and the ssdp request
    // service (2 fds). These are file descriptors summarized in the structure
    // MiniServerSockArray. For details look there.
    //
    // For this test we use only socket file descriptor miniServerSock4 that is
    // listening on IPv4 for the miniserver. --Ingo
    std::cout
        << CYEL "[ TODO     ] " CRES << __LINE__
        << ": Test must be extended for IPv6 sockets and other sockets.\n";

    // CLogging loggingObj; // Output only with build type DEBUG.

    // Initialize the threadpool. Don't forget to shutdown the threadpool at the
    // end. nullptr means to use default attributes.
    ASSERT_EQ(ThreadPoolInit(&gMiniServerThreadPool, nullptr), 0);
    // Prevent to add jobs, we test jobs isolated.
    gMiniServerThreadPool.shutdown = 1;
    // EXPECT_EQ(TPAttrSetMaxJobsTotal(&gMiniServerThreadPool.attr, 0), 0);

    // We need this on the heap because it is freed by 'RunMiniServer()'.
    MiniServerSockArray* minisock =
        (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
    ASSERT_NE(minisock, nullptr);
    InitMiniServerSockArray(minisock);

    // Set needed data, listen miniserver only on IPv4 that will connect to a
    // remote client which has done e rquest.
    minisock->miniServerPort4 = 50012;
    minisock->stopPort = 50013;
    const std::string remote_connect_port = "50014";
    // minisock->miniServerPort6 = 5xxxx;

    // Due to 'select()' have ATTENTION to set select_nfds correct.
    SOCKET select_nfds{umock::sfd_base + 8 + 1}; // Must be highest used fd + 1
    minisock->miniServerSock4 = umock::sfd_base + 6;
    // minisock->miniServerSock6 = umock::sfd_base + n;
    // minisock->ssdpSock4 = umock::sfd_base + n;
    minisock->miniServerStopSock = umock::sfd_base + 8;
    // Next is the socket file descriptor of an accepted connection to a remote
    // peer and not part of listening sockets monitored by select().
    constexpr SOCKET remote_connect_sockfd = umock::sfd_base + 9;

    { // Scope of mocking only within this block

        if (old_code) {
            std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                      << ": Max socket fd for select() not setting to 0 if "
                         "INVALID_SOCKET in MiniServerSockArray on WIN32.\n";
#ifdef _WIN32
            // On MS Windows INVALID_SOCKET is unsigned -1 =
            // 18446744073709551615 so we get select_nfds with this big number
            // even if there is only one INVALID_SOCKET. Incrementing it by one
            // results in 0. To be portable we must not assume INVALID_SOCKET
            // to be -1. --Ingo
            select_nfds = 0; // Wrong!
#endif
            EXPECT_CALL(m_sys_socketObj,
                        select(select_nfds, _, nullptr, _, nullptr))
                .WillOnce(Return(1));

        } else {

            EXPECT_CALL(m_sys_socketObj,
                        select(select_nfds, _, nullptr, _, nullptr))
                .WillOnce(Return(1));
        }

        SSockaddr_storage ss_connected;
        ss_connected = "192.168.200.201:" + remote_connect_port;
        EXPECT_CALL(m_sys_socketObj,
                    accept(minisock->miniServerSock4, NotNull(),
                           Pointee((socklen_t)sizeof(sockaddr_storage))))
            .WillOnce(DoAll(SetArgPointee<1>(*(sockaddr*)&ss_connected.ss),
                            Return(remote_connect_sockfd)));

        SSockaddr_storage ss_localhost;
        ss_localhost = "127.0.0.1:" + std::to_string(minisock->stopPort);

        if (old_code) {
            EXPECT_CALL(m_sys_socketObj,
                        recvfrom(minisock->miniServerStopSock, _, 25, 0, _,
                                 Pointee((socklen_t)sizeof(sockaddr_storage))))
                .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"),
                                SetArgPointee<4>(*(sockaddr*)&ss_localhost.ss),
                                Return((SSIZEP_T)sizeof("ShutDown"))));

        } else {

            constexpr char shutdown_str[]("ShutDown");
            constexpr size_t shutdown_strlen{sizeof(shutdown_str)};
            // It is important to expect shutdown_strlen.
            EXPECT_CALL(m_sys_socketObj,
                        recvfrom(minisock->miniServerStopSock, _,
                                 shutdown_strlen, 0, _,
                                 Pointee((socklen_t)sizeof(sockaddr_storage))))
                .WillOnce(DoAll(StrCpyToArg<1>(shutdown_str),
                                SetArgPointee<4>(*(sockaddr*)&ss_localhost.ss),
                                Return((SSIZEP_T)shutdown_strlen)));
        }

        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Unit must not expect its argument MiniServerSockArray* "
                     "to be on the heap and free it.\n";

        // Test Unit
        RunMiniServer(minisock);

    } // End scope of mocking, objects within the block will be destructed.

    // Shutdown the threadpool.
    EXPECT_EQ(ThreadPoolShutdown(&gMiniServerThreadPool), 0);
}

} // namespace compa


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
