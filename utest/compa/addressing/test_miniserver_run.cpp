// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-22

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

#include <upnplib/general.hpp>
#include <upnplib/sockaddr.hpp>

#include <utest/utest.hpp>
#include <umock/sys_socket_mock.hpp>


namespace utest {
bool old_code{false}; // Managed in gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::testing::_;
using ::testing::DoAll;
using ::testing::Ge;
using ::testing::HasSubstr;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;
using ::testing::StrictMock;

using ::upnplib::SSockaddr_storage;


// Miniserver Run TestSuite
// ========================
class MiniServerRunFTestSuite : public ::testing::Test {
    // This is a fixture to provide mocking of sys_socket only.
  protected:
    MiniServerSockArray* m_minisock{};

    // clang-format off
    // Instantiate mocking objects.
    StrictMock<umock::Sys_socketMock> m_sys_socketObj;
    // Inject the mocking objects into the tested code.
    umock::Sys_socket sys_socket_injectObj = umock::Sys_socket(&m_sys_socketObj);
    // clang-format on

    MiniServerRunFTestSuite() {
        TRACE2(this, " Construct MiniServerRunFTestSuite()")
    }

    ~MiniServerRunFTestSuite() {
        TRACE2(this, " Destruct MiniServerRunFTestSuite()")
    }
};

class RunMiniserverFuncFTestSuite : public MiniServerRunFTestSuite {
    // This is a fixture to provide mocking of sys_socket and initializing the
    // threadpool and a MiniServerSockArray on the heap to call the
    // RunMiniserver() function.
  protected:
    RunMiniserverFuncFTestSuite() {
        TRACE2(this, " Construct RunMiniserverFuncFTestSuite()")
        // nullptr means to use default attributes.
        int ret_ThreadPoolInit =
            ThreadPoolInit(&gMiniServerThreadPool, nullptr);
        if (ret_ThreadPoolInit != 0)
            throw std::runtime_error(
                "Initializing the ThreadPool fails with return number " +
                std::to_string(ret_ThreadPoolInit));
        // Prevent to add jobs, we test jobs isolated.
        gMiniServerThreadPool.shutdown = 1;
        // EXPECT_EQ(TPAttrSetMaxJobsTotal(&gMiniServerThreadPool.attr, 0), 0);

        // We need this on the heap because it is freed by 'RunMiniServer()'.
        m_minisock = (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
        if (m_minisock == nullptr)
            throw std::runtime_error("Failed to allocate memory.");
        InitMiniServerSockArray(m_minisock);
    }

    ~RunMiniserverFuncFTestSuite() {
        TRACE2(this, " Destruct RunMiniserverFuncFTestSuite()")
        // Shutdown the threadpool.
        ThreadPoolShutdown(&gMiniServerThreadPool);
    }
};


TEST_F(RunMiniserverFuncFTestSuite, RunMiniServer_successful) {
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

    // Set needed data, listen miniserver only on IPv4 that will connect to a
    // remote client which has done e rquest.
    m_minisock->miniServerPort4 = 50012;
    m_minisock->stopPort = 50013;
    const std::string remote_connect_port = "50014";
    // m_minisock->miniServerPort6 = 5xxxx;

    // Due to 'select()' have ATTENTION to set select_nfds correct.
    SOCKET select_nfds{umock::sfd_base + 8 + 1}; // Must be highest used fd + 1
    m_minisock->miniServerSock4 = umock::sfd_base + 6;
    // m_minisock->miniServerSock6 = umock::sfd_base + n;
    // m_minisock->ssdpSock4 = umock::sfd_base + n;
    m_minisock->miniServerStopSock = umock::sfd_base + 8;
    // Next is the socket file descriptor of an accepted connection to a remote
    // peer and not part of listening sockets monitored by select().
    constexpr SOCKET remote_connect_sockfd = umock::sfd_base + 9;

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

        // Check socket in fdset_if_valid() successful
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_minisock->miniServerSock4,
                                                SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // Mock that the socket fd ist bound to an address.
        SSockaddr_storage ssObj;
        ssObj = "[2001:db8::cd]:50059";
        EXPECT_CALL(
            m_sys_socketObj,
            getsockname(m_minisock->miniServerSock4, _,
                        Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                SetArgPointee<2>(static_cast<socklen_t>(sizeof(ssObj.ss))),
                Return(0)));

        // select in RunMiniServer() also succeeds
        EXPECT_CALL(m_sys_socketObj,
                    select(select_nfds, _, nullptr, _, nullptr))
            .WillOnce(Return(1));
    }

    SSockaddr_storage ss_connected;
    ss_connected = "192.168.200.201:" + remote_connect_port;
    EXPECT_CALL(m_sys_socketObj,
                accept(m_minisock->miniServerSock4, NotNull(),
                       Pointee((socklen_t)sizeof(sockaddr_storage))))
        .WillOnce(DoAll(SetArgPointee<1>(*(sockaddr*)&ss_connected.ss),
                        Return(remote_connect_sockfd)));

    SSockaddr_storage ss_localhost;
    ss_localhost = "127.0.0.1:" + std::to_string(m_minisock->stopPort);

    if (old_code) {
        EXPECT_CALL(m_sys_socketObj,
                    recvfrom(m_minisock->miniServerStopSock, _, 25, 0, _,
                             Pointee((socklen_t)sizeof(sockaddr_storage))))
            .WillOnce(DoAll(StrCpyToArg<1>("ShutDown"),
                            SetArgPointee<4>(*(sockaddr*)&ss_localhost.ss),
                            Return((SSIZEP_T)sizeof("ShutDown"))));

    } else {

        constexpr char shutdown_str[]("ShutDown");
        constexpr size_t shutdown_strlen{sizeof(shutdown_str)};
        // It is important to expect shutdown_strlen.
        EXPECT_CALL(m_sys_socketObj,
                    recvfrom(m_minisock->miniServerStopSock, _, shutdown_strlen,
                             0, _,
                             Pointee((socklen_t)sizeof(sockaddr_storage))))
            .WillOnce(DoAll(StrCpyToArg<1>(shutdown_str),
                            SetArgPointee<4>(*(sockaddr*)&ss_localhost.ss),
                            Return((SSIZEP_T)shutdown_strlen)));
    }

    std::cout << CRED "[ BUG      ] " CRES << __LINE__
              << ": Unit must not expect its argument MiniServerSockArray* "
                 "to be on the heap and free it.\n";

    // Test Unit
    RunMiniServer(m_minisock);
}

TEST_F(RunMiniserverFuncFTestSuite, RunMiniServer_select_fails_with_no_memory) {
    // See important note at
    // TEST_F(RunMiniserverFuncFTestSuite, RunMiniServer_successful).

    // Set needed data, listen miniserver only on IPv4 that will connect to a
    // remote client which has done a request.
    m_minisock->miniServerPort4 = 50025;
    m_minisock->stopPort = 50026;

    // Due to 'select()' have ATTENTION to set select_nfds correct.
    SOCKET select_nfds{umock::sfd_base + 55 + 1}; // Must be highest used fd + 1
    m_minisock->miniServerSock4 = umock::sfd_base + 54;
    m_minisock->miniServerStopSock = umock::sfd_base + 55;

    if (old_code) {
#ifdef _WIN32
        // On MS Windows INVALID_SOCKET is unsigned -1 =
        // 18446744073709551615 so we get select_nfds with this big number
        // even if there is only one INVALID_SOCKET. Incrementing it by one
        // results in 0. To be portable we must not assume INVALID_SOCKET
        // to be -1. This bug is fixed in new code. --Ingo
        select_nfds = 0; // Wrong!
#endif
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Ongoing failing select() must not hang in an "
                     "endless loop.\n";
        EXPECT_CALL(m_sys_socketObj,
                    select(select_nfds, _, nullptr, _, nullptr))
            .WillOnce(SetErrnoAndReturn(ENOMEM, SOCKET_ERROR))
            // Next is only to force errors to leave endless loop.
            .WillOnce(Return(1)); // Wrong!
        EXPECT_CALL(m_sys_socketObj, accept(_, _, _))
            .WillOnce(SetErrnoAndReturn(ENOMEM, INVALID_SOCKET));
        EXPECT_CALL(m_sys_socketObj, recvfrom(_, _, _, _, _, _))
            .WillOnce(SetErrnoAndReturn(ENOMEM, SOCKET_ERROR));

    } else {

        // Check socket in fdset_if_valid() successful
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_minisock->miniServerSock4,
                                                SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // Mock that the socket fd ist bound to an address.
        SSockaddr_storage ssObj;
        ssObj = "192.168.10.10:50060";
        EXPECT_CALL(
            m_sys_socketObj,
            getsockname(m_minisock->miniServerSock4, _,
                        Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                SetArgPointee<2>(static_cast<socklen_t>(sizeof(ssObj.ss))),
                Return(0)));

        // but select in RunMiniServer() fails
        EXPECT_CALL(m_sys_socketObj,
                    select(select_nfds, _, nullptr, _, nullptr))
            .WillOnce(SetErrnoAndReturn(ENOMEM, SOCKET_ERROR));
    }

    // Capture output to stderr
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    RunMiniServer(m_minisock);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should not silently ignore critical error "
                     "messages from ::select().\n";
        EXPECT_EQ(captureObj.str(), ""); // Wrong!

    } else {

        if (upnplib::g_dbug)
            std::cout << captureObj.str();
        EXPECT_THAT(captureObj.str(), HasSubstr("] CRITICAL MSG1021: "));
    }
}

TEST_F(MiniServerRunFTestSuite, fdset_if_valid_read_successful) {
    // Socket file descriptor should be added to the read set.
    constexpr SOCKET sockfd{umock::sfd_base + 56};

    if (!old_code) {
        // ::getsockopt() is used to verify a valid socket. Here I mock it to
        // find sockfd valid.
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // ::getsockname() is used to verify if a socket is bound to a local
        // interface address. Here I mock it to find the socket is bound.
        SSockaddr_storage ssObj;
        ssObj = "192.168.10.11:50061";
        EXPECT_CALL(
            m_sys_socketObj,
            getsockname(sockfd, _,
                        Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                SetArgPointee<2>(static_cast<socklen_t>(sizeof(ssObj.ss))),
                Return(0)));
    }

    // Test Unit
    fd_set rdSet;
    FD_ZERO(&rdSet);
    fdset_if_valid(sockfd, &rdSet);

    EXPECT_NE(FD_ISSET(sockfd, &rdSet), 0)
        << "Socket file descriptor " << sockfd
        << " should be added to the FD SET for select().";
}

TEST_F(MiniServerRunFTestSuite, fdset_if_valid_fails) {
    fd_set rdSet;
    FD_ZERO(&rdSet);
    ASSERT_FALSE(FD_ISSET(static_cast<SOCKET>(0), &rdSet));

    // Capture output to stderr
    bool dbug_old = upnplib::g_dbug;
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO

    if (old_code) {

        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should not terminate program with \"*** buffer "
                     "overflow detected ***\".\n";
        // Next cannot be tested because pupnp fdset_if_valid() does not
        // respect limit FD_SETSIZE and randomly produces "*** buffer overflow
        // detected ***: terminated"
        // FD_ZERO(&rdSet);
        // captureObj.start();
        // upnplib::g_dbug = true;
        // fdset_if_valid(static_cast<SOCKET>(-17000), &rdSet);
        // upnplib::g_dbug = dbug_old;
        // EXPECT_EQ(captureObj.str(), ""); // Wrong!

        // Test Unit, fd 0 to 2 are not valid network sockets and should not
        // be set.
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should not accept file descriptors 0 to 2 or >= "
                     "FD_SETSIZE.\n";
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(0), &rdSet);
        EXPECT_TRUE(FD_ISSET(static_cast<SOCKET>(0), &rdSet)); // Wrong!
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(1), &rdSet);
        EXPECT_TRUE(FD_ISSET(static_cast<SOCKET>(1), &rdSet)); // Wrong!
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(2), &rdSet);
        EXPECT_TRUE(FD_ISSET(static_cast<SOCKET>(2), &rdSet)); // Wrong!
        // Next cannot be tested because pupnp fdset_if_valid() does not respect
        // limit FD_SETSIZE and randomly produces "*** buffer overflow detected
        // ***: terminated"
        // FD_ZERO(&rdSet);
        // captureObj.start();
        // upnplib::g_dbug = true;
        // fdset_if_valid(static_cast<SOCKET>(FD_SETSIZE), &rdSet);
        // upnplib::g_dbug = dbug_old;
        // EXPECT_EQ(captureObj.str(), ""); // Wrong!

    } else { // if (old_code)

        // Test Unit
        FD_ZERO(&rdSet);
        captureObj.start();
        upnplib::g_dbug = true;
        fdset_if_valid(static_cast<SOCKET>(-17000), &rdSet);
        upnplib::g_dbug = dbug_old;

        // Get captured output
        if (upnplib::g_dbug)
            std::cout << captureObj.str();
        EXPECT_THAT(captureObj.str(), HasSubstr("] ERROR MSG1005: "));

        // Test Unit
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(0), &rdSet);
        EXPECT_FALSE(FD_ISSET(static_cast<SOCKET>(0), &rdSet));
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(1), &rdSet);
        EXPECT_FALSE(FD_ISSET(static_cast<SOCKET>(1), &rdSet));
        FD_ZERO(&rdSet);
        fdset_if_valid(static_cast<SOCKET>(2), &rdSet);
        EXPECT_FALSE(FD_ISSET(static_cast<SOCKET>(2), &rdSet));
        FD_ZERO(&rdSet);
        captureObj.start();
        upnplib::g_dbug = true;
        fdset_if_valid(static_cast<SOCKET>(FD_SETSIZE), &rdSet);
        upnplib::g_dbug = dbug_old;

        // Get captured output
        if (upnplib::g_dbug)
            std::cout << captureObj.str();
        EXPECT_THAT(captureObj.str(),
                    HasSubstr("] ERROR MSG1005: Prohibited socket 1024"));
    }
}

TEST_F(MiniServerRunFTestSuite, fdset_if_valid_fails_with_invalid_socket) {
    // Provide a socket file descriptor.
    constexpr SOCKET sockfd{umock::sfd_base + 7};

    if (!old_code) {
        // ::getsockopt() is used to verify a valid socket. Here I mock it to
        // find sockfd invalid.
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(SetErrnoAndReturn(EBADF, SOCKET_ERROR));
    }

    // Test Unit
    fd_set rdSet;
    FD_ZERO(&rdSet);
    fdset_if_valid(sockfd, &rdSet);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Socket file descriptor " << sockfd
                  << " should not be added to the FD SET for ::select().\n";
        EXPECT_NE(FD_ISSET(sockfd, &rdSet), 0); // Wrong!

    } else {

        EXPECT_EQ(FD_ISSET(sockfd, &rdSet), 0)
            << "Socket file descriptor " << sockfd
            << " should not be added to the FD SET for ::select().";
    }
}

TEST_F(MiniServerRunFTestSuite, fdset_if_valid_fails_with_unbind_socket) {
    // Provide a socket file descriptor.
    constexpr SOCKET sockfd{umock::sfd_base + 53};

    if (!old_code) {
        // ::getsockopt() is used to verify a valid socket. Here I mock it to
        // find sockfd valid.
        EXPECT_CALL(m_sys_socketObj,
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // ::getsockname() is used to verify if a socket is bound to a local
        // interface address. Here I mock it with the unknown sockaddr "[::]"
        // to find the socket is not bound.
        SSockaddr_storage ssObj;
        ssObj = "[::]";
        EXPECT_CALL(
            m_sys_socketObj,
            getsockname(sockfd, _,
                        Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                SetArgPointee<2>(static_cast<socklen_t>(sizeof(ssObj.ss))),
                Return(0)));
    }

    // Test Unit
    fd_set rdSet;
    FD_ZERO(&rdSet);
    fdset_if_valid(sockfd, &rdSet);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Socket file descriptor " << sockfd
                  << " should not be added to the FD SET for ::select().\n";
        EXPECT_NE(FD_ISSET(sockfd, &rdSet), 0); // Wrong!

    } else {

        EXPECT_EQ(FD_ISSET(sockfd, &rdSet), 0)
            << "Socket file descriptor " << sockfd
            << " should not be added to the FD SET for ::select().";
    }
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "gtest_main.inc"
    return gtest_return_code; // managed in gtest_main.inc
}
