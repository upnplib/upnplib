// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-07

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

#include <pupnp/upnpdebug.hpp>
#include <pupnp/threadpool_init.hpp>

#include <utest/utest.hpp>
#include <umock/sys_socket_mock.hpp>


namespace utest {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Ge;
using ::testing::HasSubstr;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;
using ::testing::SetErrnoAndReturn;
using ::testing::StrictMock;

using ::upnplib::g_dbug;
using ::upnplib::SSockaddr_storage;

using ::pupnp::CLogging;
using ::pupnp::CThreadPoolInit;


// Miniserver Run TestSuite
// ========================
class MiniServerMockFTestSuite : public ::testing::Test {
    // This is a fixture to provide mocking of sys_socket.
  protected:
    // clang-format off
    // Instantiate mocking objects.
    StrictMock<umock::Sys_socketMock> m_sys_socketObj;
    // Inject the mocking objects into the tested code.
    umock::Sys_socket sys_socket_injectObj = umock::Sys_socket(&m_sys_socketObj);
    // clang-format on

    MiniServerMockFTestSuite() {
        TRACE2(this, " Construct MiniServerMockFTestSuite()")
    }

    virtual ~MiniServerMockFTestSuite() override {
        TRACE2(this, " Destruct MiniServerMockFTestSuite()")
    }
};

class RunMiniServerFuncFTestSuite : public MiniServerMockFTestSuite {
    // This is a fixture to provide mocking of sys_socket and a
    // MiniServerSockArray on the heap to call the RunMiniserver() function.
    // This is needed because RunMiniserver() frees MiniServerSockArray.
  protected:
    MiniServerSockArray* m_minisock{};

    RunMiniServerFuncFTestSuite() {
        TRACE2(this, " Construct RunMiniServerFuncFTestSuite()")
        // We need this on the heap because it is freed by 'RunMiniServer()'.
        m_minisock = (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
        if (m_minisock == nullptr)
            throw std::runtime_error("Failed to allocate memory.");
        InitMiniServerSockArray(m_minisock);
    }

    virtual ~RunMiniServerFuncFTestSuite() override {
        TRACE2(this, " Destruct RunMiniServerFuncFTestSuite()")
    }
};


TEST_F(RunMiniServerFuncFTestSuite, RunMiniServer_successful) {
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

    CLogging logObj; // Output only with build type DEBUG.
    if (g_dbug)
        logObj.enable(UPNP_ALL);

    // With shutdown = true, maxJobs is ignored.
    std::cout
        << CYEL "[ TODO     ] " CRES << __LINE__
        << ": Fix dynamic problem with shutdown before job initialization.\n";
    // CThreadPoolInit tp(gMiniServerThreadPool,
    //                    /*shutdown*/ false, /*maxJobs*/ 1);
    // Workaround:
    CThreadPoolInit tp(gMiniServerThreadPool,
                       /*shutdown*/ true); //*maxJobs*/ 1);

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
    // host and not part of listening sockets monitored by select().
    constexpr SOCKET remote_connect_sockfd = umock::sfd_base + 9;

    // We need this to mock socket addresses.
    SSockaddr_storage ssObj;

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

        std::cout << CYEL "[ TODO     ] " CRES << __LINE__
                  << ": Unit must not expect its argument MiniServerSockArray* "
                     "to be on the heap and free it.\n";

        // Check socket in fdset_if_valid() successful
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_minisock->miniServerSock4,
                                                SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // Mock that the socket fd ist bound to an address of a local interface.
        ssObj = "[2001:db8::cd]:50059";
        EXPECT_CALL(m_sys_socketObj, getsockname(m_minisock->miniServerSock4, _,
                                                 Pointee(ssObj.get_sslen())))
            .WillOnce(
                DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                      SetArgPointee<2>(ssObj.get_sslen()), Return(0)));

        // select() in RunMiniServer() also succeeds.
        EXPECT_CALL(m_sys_socketObj,
                    select(select_nfds, _, nullptr, _, nullptr))
            .WillOnce(Return(1));
    }

    // accept() in RunMiniServer() succeeds and returns the remote ip address
    // that it is connected to.
    ssObj = "192.168.200.201:" + remote_connect_port;
    EXPECT_CALL(m_sys_socketObj,
                accept(m_minisock->miniServerSock4, NotNull(),
                       Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
        .WillOnce(
            DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(remote_connect_sockfd)));

    // Here I mock receiving of the ShutDown datagram.
    constexpr char shutdown_str[]("ShutDown");
    constexpr SIZEP_T shutdown_strlen{sizeof(shutdown_str)};
    ssObj = "127.0.0.1:" + std::to_string(m_minisock->stopPort);
    // It is important to expect shutdown_strlen.
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(m_minisock->miniServerStopSock, _, Ge(shutdown_strlen),
                         0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrCpyToArg<1>(shutdown_str),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(shutdown_strlen)));

    // Test Unit
    RunMiniServer(m_minisock);
}

TEST_F(RunMiniServerFuncFTestSuite, RunMiniServer_select_fails_with_no_memory) {
    // See important note at
    // TEST_F(RunMiniServerFuncFTestSuite, RunMiniServer_successful).

    CLogging logObj; // Output only with build type DEBUG.
    if (g_dbug)
        logObj.enable(UPNP_ALL);

    // With shutdown = true, maxJobs is ignored.
    CThreadPoolInit tp(gMiniServerThreadPool,
                       /*shutdown*/ false, /*maxJobs*/ 1);

    // Set needed data, listen miniserver only on IPv4 that will connect to a
    // remote client which has done a request.
    m_minisock->miniServerPort4 = 50025;
    m_minisock->stopPort = 50026;

    // Due to 'select()' have ATTENTION to set select_nfds correct.
    SOCKET select_nfds{umock::sfd_base + 55 + 1}; // Must be highest used fd + 1
    m_minisock->miniServerSock4 = umock::sfd_base + 54;
    m_minisock->miniServerStopSock = umock::sfd_base + 55;

    // We need this to mock socket addresses.
    SSockaddr_storage ssObj;

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

        // Mock that the socket fd ist bound to an address of a local interface.
        ssObj = "192.168.10.10:50060";
        EXPECT_CALL(m_sys_socketObj, getsockname(m_minisock->miniServerSock4, _,
                                                 Pointee(ssObj.get_sslen())))
            .WillOnce(
                DoAll(SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                      SetArgPointee<2>(ssObj.get_sslen()), Return(0)));

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
    std::cout << captureObj.str();

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Unit should critical errors always output, not only "
                     "with build type DEBUG and no garbage.\n";
        if (g_dbug)
#ifdef UPNP_HAVE_DEBUG
            EXPECT_THAT(captureObj.str(),
                        HasSubstr("stopSock Error, aborting..."));
#else
            EXPECT_EQ(captureObj.str(), ""); // Wrong!
#endif
        else
            EXPECT_EQ(captureObj.str(), ""); // Wrong!

    } else { // New code

        EXPECT_THAT(captureObj.str(), HasSubstr("] CRITICAL MSG1021: "));
    }
}

TEST_F(RunMiniServerFuncFTestSuite, RunMiniServer_accept_fails) {
    // See important note at
    // TEST_F(RunMiniServerFuncFTestSuite, RunMiniServer_successful).
    // For this test I use only socket file descriptor miniServerSock4 that is
    // listening on IPv4 for the miniserver. --Ingo

    CLogging logObj; // Output only with build type DEBUG.
    logObj.enable(UPNP_ALL);

    // With shutdown = true, maxJobs is ignored.
    CThreadPoolInit tp(gMiniServerThreadPool,
                       /*shutdown*/ false, /*maxJobs*/ 1);

    // Set needed data, listen miniserver only on IPv4 that will connect to a
    // remote client which has done a request.
    m_minisock->miniServerPort4 = 50045;
    m_minisock->stopPort = 50047;

    // Due to 'select()' have ATTENTION to set select_nfds correct.
    SOCKET select_nfds{umock::sfd_base + 59 + 1}; // Must be highest used fd + 1
    m_minisock->miniServerSock4 = umock::sfd_base + 58;
    m_minisock->miniServerStopSock = umock::sfd_base + 59;

    constexpr char shutdown_str[]{"ShutDown"};
    SIZEP_T shutdown_strlen;

    if (old_code) {
#ifdef _WIN32
        // On MS Windows INVALID_SOCKET is unsigned -1 =
        // 18446744073709551615 so we get select_nfds with this big number
        // even if there is only one INVALID_SOCKET. Incrementing it by one
        // results in 0. To be portable we must not assume INVALID_SOCKET
        // to be -1. --Ingo
        select_nfds = 0; // Wrong!
#endif
        shutdown_strlen = 25; // This is fixed given by the tested Unit
    } else {
        shutdown_strlen = sizeof(shutdown_str);
    }

    // select()
    if (!old_code) {
        // Mock socket check in 'fdset_if_valid()' to be successful.
        // 'getsockopt()' and 'getsockname()' are called in the production
        // code for verification.
        EXPECT_CALL(m_sys_socketObj, getsockopt(m_minisock->miniServerSock4,
                                                SOL_SOCKET, SO_ERROR, _, _))
            .WillOnce(Return(0));

        // Mock that the socket fd ist bound to an address.
        SSockaddr_storage ssObj;
        ssObj = "[2001:db8::ab]:50044";
        EXPECT_CALL(
            m_sys_socketObj,
            getsockname(m_minisock->miniServerSock4, _,
                        Pointee(Ge(static_cast<socklen_t>(sizeof(ssObj.ss))))))
            .WillOnce(DoAll(
                SetArgPointee<1>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                SetArgPointee<2>(static_cast<socklen_t>(sizeof(ssObj.ss))),
                Return(0)));
    }

    // select in RunMiniServer() also succeeds.
    EXPECT_CALL(m_sys_socketObj, select(select_nfds, _, nullptr, _, nullptr))
        .WillOnce(Return(2)); // data and stopsock available on select()

    // accept() will fail for incomming data. stopsock uses a datagram so it
    // doesn't use accept().
    EXPECT_CALL(m_sys_socketObj,
                accept(m_minisock->miniServerSock4, NotNull(),
                       Pointee((socklen_t)sizeof(sockaddr_storage))))
        .WillOnce(SetErrnoAndReturn(ENOMEM, INVALID_SOCKET));

    // Provide data for stopsock with ShutDown.
    SSockaddr_storage ss_localhost;
    ss_localhost = "127.0.0.1:" + std::to_string(m_minisock->stopPort);

    EXPECT_CALL(m_sys_socketObj,
                recvfrom(m_minisock->miniServerStopSock, _, shutdown_strlen, 0,
                         _, Pointee((socklen_t)sizeof(sockaddr_storage))))
        .WillOnce(DoAll(StrCpyToArg<1>(shutdown_str),
                        SetArgPointee<4>(*(sockaddr*)&ss_localhost.ss),
                        Return(shutdown_strlen)));

    // Test Unit
    RunMiniServer(m_minisock);
}

TEST_F(MiniServerMockFTestSuite, fdset_if_valid_read_successful) {
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

TEST_F(MiniServerMockFTestSuite, fdset_if_valid_fails) {
    fd_set rdSet;
    FD_ZERO(&rdSet);
    ASSERT_FALSE(FD_ISSET(static_cast<SOCKET>(0), &rdSet));

    // Capture output to stderr
    bool dbug_old = g_dbug;
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
        // g_dbug = true;
        // fdset_if_valid(static_cast<SOCKET>(-17000), &rdSet);
        // g_dbug = dbug_old;
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
        // g_dbug = true;
        // fdset_if_valid(static_cast<SOCKET>(FD_SETSIZE), &rdSet);
        // g_dbug = dbug_old;
        // EXPECT_EQ(captureObj.str(), ""); // Wrong!

    } else { // if (old_code)

        // Test Unit
        FD_ZERO(&rdSet);
        captureObj.start();
        g_dbug = true;
        fdset_if_valid(static_cast<SOCKET>(-17000), &rdSet);
        g_dbug = dbug_old;

        // Get captured output
        if (g_dbug)
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
        g_dbug = true;
        fdset_if_valid(static_cast<SOCKET>(FD_SETSIZE), &rdSet);
        g_dbug = dbug_old;

        // Get captured output
        if (g_dbug)
            std::cout << captureObj.str();
        EXPECT_THAT(captureObj.str(),
                    HasSubstr("] ERROR MSG1005: Prohibited socket 1024"));
    }
}

TEST_F(MiniServerMockFTestSuite, fdset_if_valid_fails_with_invalid_socket) {
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

TEST_F(MiniServerMockFTestSuite, fdset_if_valid_fails_with_unbind_socket) {
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

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_successful) {
    // The stop socket is got with 'get_miniserver_stopsock()' and uses a
    // datagram with exactly "ShutDown" on AF_INET to 127.0.0.1.
    constexpr char shutdown_str[]{"ShutDown"};
    // This should be the buffer size in the tested code. The test will fail if
    // it does not match so there is no danger to break destination buffer size.
    constexpr SIZEP_T expected_destbuflen{sizeof(shutdown_str)};

    constexpr SOCKET sockfd{umock::sfd_base + 5};
    SSockaddr_storage ssObj;
    ssObj = "127.0.0.1:50015";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    // expected_destbuflen is important here to avoid buffer limit overwrite.
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(sockfd, _, Ge(expected_destbuflen), 0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrnCpyToArg<1>(shutdown_str, expected_destbuflen),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(expected_destbuflen)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 1);
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_not_selected) {
    constexpr SOCKET sockfd{umock::sfd_base + 29};

    fd_set rdSet;
    FD_ZERO(&rdSet);
    // Socket not selected to be received
    // FD_SET(sockfd, &rdSet);
    // This should not call recvfrom() and is guarded by StrictMock<>.

    // Capture output to stderr
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    bool dbug_old = g_dbug;

    // Test Unit should silently nothing have received.
    captureObj.start();
    g_dbug = true;
    EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 0);
    g_dbug = dbug_old;

    EXPECT_EQ(captureObj.str(), "");
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_receiving_fails) {
    constexpr SOCKET sockfd{umock::sfd_base + 28};
    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj, recvfrom(sockfd, _, _, _, _, _))
        .WillOnce(SetErrnoAndReturn(EINVAL, SOCKET_ERROR));

    // Test Unit
    // Here we have a critical error. The Unit should return 'true' to stop the
    // miniserver.
    EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 1);
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_no_bytes_received) {
    CLogging logObj; // Output only with build type DEBUG.
    if (old_code)
        if (g_dbug)
            logObj.enable(UPNP_ALL);

    constexpr char shutdown_str[]{""};
    constexpr SIZEP_T bufsizeof_ShutDown_str{9};
    constexpr SOCKET sockfd{umock::sfd_base + 30};
    SSockaddr_storage ssObj;
    ssObj = "127.0.0.1:50015";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(sockfd, _, Ge(bufsizeof_ShutDown_str), 0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrCpyToArg<1>(shutdown_str),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(0)));

    // Test Unit
    // Returns 1 (true) if successfully received "ShutDown" from stopSock
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ] " CRES << __LINE__
            << ": Receiving a datagram with 0 bytes length should fail.\n";
        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 1); // Wrong!

    } else {

        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 0);
    }
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_wrong_stop_message) {
    CLogging logObj;
    if (old_code)
        if (g_dbug)
            logObj.enable(UPNP_ALL);

    constexpr char shutdown_str[]{"Nothings"};
    // This should be the buffer size in the tested code. The test will fail if
    // it does not match so there is no danger to break buffer size.
    constexpr SIZEP_T expected_destbuflen{sizeof(shutdown_str)};

    constexpr SOCKET sockfd{umock::sfd_base + 31};
    SSockaddr_storage ssObj;
    ssObj = "127.0.0.1:50017";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    // expected_destbuflen is important here to avoid buffer limit overwrite.
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(sockfd, _, Ge(expected_destbuflen), 0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrnCpyToArg<1>(shutdown_str, expected_destbuflen),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(expected_destbuflen)));

    // Test Unit
    EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 0);
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_from_wrong_address) {
    CLogging logObj; // Output only with build type DEBUG.
    if (old_code)
        if (g_dbug)
            logObj.enable(UPNP_ALL);

    constexpr char shutdown_str[]{"ShutDown"};
    // This should be the buffer size in the tested code. The test will fail if
    // it does not match so there is no danger to break buffer size.
    constexpr SIZEP_T expected_destbuflen{sizeof(shutdown_str)};

    constexpr SOCKET sockfd{umock::sfd_base + 48};
    SSockaddr_storage ssObj;
    ssObj = "192.168.150.151:50018";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    // expected_destbuflen is important here to avoid buffer limit overwrite.
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(sockfd, _, Ge(expected_destbuflen), 0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrnCpyToArg<1>(shutdown_str, expected_destbuflen),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(expected_destbuflen)));

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Receiving ShutDown not from 127.0.0.1 must fail.\n";
        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 1); // Wrong!

    } else {

        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 0);
    }
}

TEST_F(MiniServerMockFTestSuite, receive_from_stopsock_without_0_termbyte) {
    CLogging logObj; // Output only with build type DEBUG.
    if (old_code)
        if (g_dbug)
            logObj.enable(UPNP_ALL);

    constexpr char shutdown_str[]{"ShutDown "};
    // With this buffer size the last byte is a space but not a '\0'.
    constexpr SIZEP_T expected_destbuflen{sizeof(shutdown_str) - 1};

    constexpr SOCKET sockfd{umock::sfd_base + 49};
    SSockaddr_storage ssObj;
    ssObj = "127.0.0.1:50019";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(sockfd, &rdSet);

    // Mock system functions
    // expected_destbuflen is important here to avoid buffer limit overwrite.
    EXPECT_CALL(m_sys_socketObj,
                recvfrom(sockfd, _, Ge(expected_destbuflen), 0, _,
                         Pointee(static_cast<socklen_t>(sizeof(ssObj.ss)))))
        .WillOnce(
            DoAll(StrnCpyToArg<1>(shutdown_str, expected_destbuflen),
                  SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ssObj.ss)),
                  Return(expected_destbuflen)));

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Receiving \"ShutDown\" without terminating nullbyte "
                     "must fail.\n";
        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 1); // Wrong!

    } else {

        EXPECT_EQ(receive_from_stopSock(sockfd, &rdSet), 0);
    }
}

TEST_F(MiniServerMockFTestSuite, ssdp_read_successful) {
    CLogging logObj; // Output only with build type DEBUG.
    if (g_dbug)
        logObj.enable(UPNP_ALL);

    // Initialize the Recv Threadpool, allow only one job.
    // With shutdown = true, maxJobs is ignored.
    CThreadPoolInit tp(gRecvThreadPool,
                       /*shutdown*/ false, /*maxJobs*/ 1);

    constexpr char ssdpdata_str[]{
        "Some SSDP test data for a request of a remote client."};
    ASSERT_LE(sizeof(ssdpdata_str), BUFSIZE - 1);
    const char* const ssdpd_str_last{ssdpdata_str + sizeof(ssdpdata_str)};

    SOCKET ssdp_sockfd{umock::sfd_base + 32};
    SSockaddr_storage ss;
    ss = "192.168.71.82:50023";

    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(ssdp_sockfd, &rdSet);

    EXPECT_CALL(m_sys_socketObj,
                recvfrom(ssdp_sockfd, _, BUFSIZE - 1, 0, _,
                         Pointee((socklen_t)sizeof(::sockaddr_storage))))
        .WillOnce(DoAll(SetArrayArgument<1>(ssdpdata_str, ssdpd_str_last),
                        SetArgPointee<4>(*reinterpret_cast<sockaddr*>(&ss.ss)),
                        Return((SSIZEP_T)sizeof(ssdpdata_str))));

    // Capture output to stderr
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    ssdp_read(&ssdp_sockfd, &rdSet); // Will return an error to ssdp_sockfd

    EXPECT_THAT(
        captureObj.str(),
        AnyOf("",
              HasSubstr(
                  "Some SSDP test data for a request of a remote client.")));
    EXPECT_NE(ssdp_sockfd, INVALID_SOCKET);
}

TEST_F(MiniServerMockFTestSuite, ssdp_read_fails) {
    // Due to error there is no job added to the Threadpool. Initializing it
    // is not needed.

    CLogging logObj; // Output only with build type DEBUG.
    if (g_dbug)
        logObj.enable(UPNP_ALL);

    constexpr char ssdpdata_str[]{
        "Some SSDP test data for a request of a remote client."};
    ASSERT_LE(sizeof(ssdpdata_str), BUFSIZE - 1);

    constexpr SOCKET ssdp_sockfd_valid{umock::sfd_base + 46};

    // Instantiate mocking objects.
    EXPECT_CALL(m_sys_socketObj, recvfrom(_, _, _, _, _, _)).Times(0);

    // Socket is set but not in the select set.
    SOCKET ssdp_sockfd{ssdp_sockfd_valid};
    fd_set rdSet;
    FD_ZERO(&rdSet);

    // Test Unit, socket should be untouched.
    ssdp_read(&ssdp_sockfd, &rdSet);
    EXPECT_EQ(ssdp_sockfd, ssdp_sockfd_valid);

    // Invalid socket must not be set in the select set.
    ssdp_sockfd = INVALID_SOCKET;

    // Test Unit
    ssdp_read(&ssdp_sockfd, &rdSet);
    EXPECT_EQ(ssdp_sockfd, INVALID_SOCKET);

    // Socket is set, also in the select set, but reading from socket fails.
    ssdp_sockfd = ssdp_sockfd_valid;
    FD_SET(ssdp_sockfd, &rdSet);

    EXPECT_CALL(m_sys_socketObj, recvfrom(ssdp_sockfd, _, _, _, _, _))
        .WillOnce(SetErrnoAndReturn(EINVAL, SOCKET_ERROR));

    // Capture output to stderr
    CaptureStdOutErr captureObj(STDERR_FILENO); // or STDOUT_FILENO
    captureObj.start();

    // Test Unit
    ssdp_read(&ssdp_sockfd, &rdSet);

    EXPECT_THAT(
        captureObj.str(),
        AnyOf("", HasSubstr("]: miniserver: Error in readFromSSDPSocket(46): "
                            "closing socket")));
    EXPECT_EQ(ssdp_sockfd, INVALID_SOCKET);
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "utest/utest_main.inc"
    return gtest_return_code; // managed in gtest_main.inc
}
