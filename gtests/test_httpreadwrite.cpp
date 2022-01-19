// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-19

#include "gmock/gmock.h"
#include "upnplib_gtest_tools.hpp"

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp"
#include "core/src/genlib/net/http/httpreadwrite.cpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

namespace upnplib {
bool old_code{false};

//
// ######################################
// Mocked system calls
// ######################################
// See the respective include files in upnp/include/upnpmock/

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

    MOCK_METHOD(int, connect,
                (int sockfd, const struct sockaddr* addr, socklen_t addrlen),
                (override));
    MOCK_METHOD(int, getsockopt,
                (int sockfd, int level, int optname, UPNPLIB_VOID_CHAR* optval,
                 socklen_t* optlen),
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

class Mock_pupnp : public Bpupnp {
    // Class to mock the free system functions.
    Bpupnp* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_pupnp() {
        m_oldptr = pupnp;
        pupnp = this;
    }
    virtual ~Mock_pupnp() override { pupnp = m_oldptr; }

    MOCK_METHOD(int, sock_make_no_blocking, (SOCKET sock), (override));
    MOCK_METHOD(int, sock_make_blocking, (SOCKET sock), (override));
    MOCK_METHOD(int, Check_Connect_And_Wait_Connection,
                (SOCKET sock, int connect_res), (override));
};

//
// ######################################
// testsuite for Ip4 httpreadwrite
// ######################################
#if false
TEST(HttpreadwriteIp4TestSuite, Check_Connect_And_Wait_Connection_real)
// This is for humans only to check on a Unix operating system how the Unit
// works in realtime so we can correct mock it. Don't set '#if true' permanently
// because it connects to the real internet and may slow down this gtest
// dramatically, in particular with a bad or no connection. You may change the
// ip address if google.com changed it.
//
// Helpful information: [Blocking vs. non-blocking sockets]
// (https://www.scottklement.com/rpg/socktut/nonblocking.html)
{
    // Get a TCP socket
    int sockfd;
    ASSERT_NE(sockfd = ::socket(AF_INET, SOCK_STREAM, 0), -1)
        << ::strerror(errno);

    // Fill an address structure
    ::sockaddr_in saddrin{};
    saddrin.sin_family = AF_INET;
    saddrin.sin_port = htons(80);
    // This was a valid ip address from google.com
    saddrin.sin_addr.s_addr = ::inet_addr("172.217.18.110");

    // disable blocking of the connection, means 'connect()' will not wait until
    // the connection persists.
    ASSERT_EQ(sock_make_no_blocking(sockfd), 0);

    // Connect to the server
    int connect_returned;
    ASSERT_EQ(connect_returned = ::connect(sockfd, (const sockaddr*)&saddrin,
                                           sizeof(struct sockaddr_in)), -1);
    // 'connect()' does not wait because of no blocking mode. Instead it returns
    // immediately with the message 'Operation now in progress':
    int connect_errno = errno; // To be on the safe side
    EXPECT_EQ(errno, EINPROGRESS)
        << "  # Should be EINPROGRESS(" << EINPROGRESS << ")='"
        << ::strerror(EINPROGRESS) << "' but not '" << ::strerror(errno) << "'("
        << errno << ").";

    // Test the Unit
    errno = connect_errno; // errno from 'connect()' is checked by the Unit
    if (old_code) {
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(sockfd, connect_returned),
                  0)
            << ::strerror(errno);
    } else {
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(sockfd,
                                                             connect_returned),
                  0)
            << ::strerror(errno);
    }

    EXPECT_EQ(::close(sockfd), 0);
}
#endif

TEST(ClientConnectionIp4TestSuite, private_connect_successful) {
    // This file descriptor is assumed to be valid.
    int socketfd{513};

    // Fill an address structure
    ::sockaddr_in saddrin{};
    saddrin.sin_family = AF_INET;
    saddrin.sin_port = htons(80);
    saddrin.sin_addr.s_addr = ::inet_addr("192.168.192.168");

    // Steps as given by the Unit:
    // 1. sock_make_no_blocking
    // 2. try to connect
    // 3. check connection an and wait until connected or timed out
    // 4. sock_make_blocking

    // Configure expected system calls:
    // * make no blocking succeeds
    // + starting connection (no wait) returns with -1, errno = EINPROGRESS;
    // * connection succeeds
    // * make blocking succeeds

    Mock_pupnp mock_pupnpObj{};
    // First unblock connection, means don't wait on connect and return
    // immediately.
    EXPECT_CALL(mock_pupnpObj, sock_make_no_blocking(socketfd))
        .WillOnce(Return(0));

    // Then connect to the given ip address. With unblocking this will return
    // with an error condition and errno = EINPROGRESS
    Mock_sys_socket mock_socketObj{};
    EXPECT_CALL(mock_socketObj,
                connect(socketfd, (sockaddr*)&saddrin, sizeof(saddrin)))
        .WillOnce(Return(-1));

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must return value of connect().
    EXPECT_CALL(mock_pupnpObj, Check_Connect_And_Wait_Connection(socketfd, -1))
        .WillOnce(Return(0));
    // Set blocking mode
    EXPECT_CALL(mock_pupnpObj, sock_make_blocking(socketfd))
        .WillOnce(Return(0));

    // Test the Unit
    errno = EINPROGRESS;
    // errno = ENETUNREACH; // Network is unreachable.
    EXPECT_EQ(
        private_connect(socketfd, (sockaddr*)&saddrin, sizeof(sockaddr_in)), 0);
}

#if false
TEST(ClientConnectionIp4TestSuite, private_connect_no_blocking_fails) {
    // This file descriptor is assumed to be valid.
    int socketfd{513};

    // Fill an address structure
    ::sockaddr_in saddrin{};
    saddrin.sin_family = AF_INET;
    saddrin.sin_port = htons(443);
    saddrin.sin_addr.s_addr = ::inet_addr("192.168.192.168");

    // Configure expected system calls:
    // * make no blocking fails
    // * errno preset with EINVAL
    // * connect() (blocking) returns with 0, errno untouched
    // * connection succeeds
    // * make blocking succeeds

    Mock_pupnp mock_pupnpObj{};
    // First unblock connection, means don't wait on connect and return
    // immediately. Returns with error, preset errno = EINVAL.
    EXPECT_CALL(mock_pupnpObj, sock_make_no_blocking(socketfd))
        .WillOnce(SetErrnoAndReturn(
            EINVAL, 4698)); // On MS Windows are big error numbers

    // Then connect to the given ip address. With unblocking this will return
    // with an error condition and errno = EINPROGRESS. But in this case with
    // failed unblock we expect a blocked but successful connection.
    Mock_sys_socket mock_socketObj{};
    if (old_code) {
        EXPECT_CALL(mock_socketObj,
                    connect(socketfd, (sockaddr*)&saddrin, sizeof(saddrin)))
            .WillOnce(Return(0));
    } else {
        EXPECT_CALL(mock_socketObj,
                    connect(socketfd, (sockaddr*)&saddrin, sizeof(saddrin)))
            .Times(0);
    }

    // Check the connection with a short timeout (default 5s) and return if
    // ready to send. Arg1 (0 based) must be set to the return value of
    // connect().
    if (old_code) {
        EXPECT_CALL(mock_pupnpObj,
                    Check_Connect_And_Wait_Connection(socketfd, 0))
            .WillOnce(Return(0));
    } else {
        EXPECT_CALL(mock_pupnpObj,
                    Check_Connect_And_Wait_Connection(socketfd, 0))
            .Times(0);
    }

    // Set blocking mode
    EXPECT_CALL(mock_pupnpObj, sock_make_blocking(socketfd))
        .WillOnce(Return(0));

    // Test the Unit
    // errno = ENETUNREACH; // Network is unreachable.
    if (old_code) {
        EXPECT_EQ(
            private_connect(socketfd, (sockaddr*)&saddrin, sizeof(sockaddr_in)),
            0);
    } else {
        EXPECT_NE(
            private_connect(socketfd, (sockaddr*)&saddrin, sizeof(sockaddr_in)),
            0);
    }
}
#endif

TEST(HttpreadwriteIp4TestSuite, open_http_connection_to_local_ip_address) {
    // The handle will be allocated on memory by the function and the pointer
    // to it is returned here. As documented we must free it.
    // We can use a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with
    // type cast on the argument.
    http_connection_handle_t* phandle;

    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection("http://127.0.0.1",
                                                       (void**)&phandle, 3);
// TODO: Check why failing is different
#ifdef _WIN32
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << "  # Should be UPNP_E_SOCKET_ERROR(" << UPNP_E_SOCKET_ERROR
        << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
        << ").";
#else
    EXPECT_EQ(returned, UPNP_E_SOCKET_CONNECT)
        << "  # Should be UPNP_E_SOCKET_CONNECT(" << UPNP_E_SOCKET_CONNECT
        << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
        << ").";
#endif

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}

//
// ######################################
// testsuite for Ip6 httpreadwrite
// ######################################
TEST(HttpreadwriteIp6TestSuite, open_http_connection_with_ip_address) {
    // The handle will be allocated on memory by the function and the pointer
    // to it is returned here. As documented we must free it.
    // We can use a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with
    // type cast on the argument.
    http_connection_handle_t* phandle;

    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection(
        // This is the ip address from http://google.com
        "http://2a00:1450:4001:80e::200e", (void**)&phandle, 3);
    if (old_code) {
        ::std::cout << "[ BUG!     ] Connecting with an ip6 address should be "
                       "possible\n";
        EXPECT_EQ(returned, UPNP_E_INVALID_URL)
            << "  # Should be UPNP_E_INVALID_URL(" << UPNP_E_INVALID_URL
            << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
            << ").";
    } else {
        GTEST_SKIP()
            << "[  FIXIT   ] Connecting with an ip6 address should be possible";
        EXPECT_EQ(returned, UPNP_E_SUCCESS)
            << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS << ") but not "
            << UpnpGetErrorMessage(returned) << '(' << returned << ").";
    }

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}

// ######################################
// testsuite for statcodes
// ######################################
TEST(StatcodesTestSuite, http_get_code_text) {
    // const char* code_text = ::http_get_code_text(HTTP_NOT_FOUND);
    // ::std::cout << "code_text: " << code_text << ::std::endl;
    EXPECT_STREQ(::http_get_code_text(HTTP_NOT_FOUND), "Not Found");
    EXPECT_EQ(::http_get_code_text(99), nullptr);
    EXPECT_EQ(::http_get_code_text(600), nullptr);
}

//
// ######################################
// Tests for Microsoft Windows #
// ######################################
#ifdef _WIN32

// Mocked system calls on MS Windows
// ---------------------------------
class Mock_winsock2 : public Bwinsock2 {
    // Class to mock the free system functions.
    Bwinsock2* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_winsock2() {
        m_oldptr = winsock2_h;
        winsock2_h = this;
    }
    virtual ~Mock_winsock2() override { winsock2_h = m_oldptr; }

    MOCK_METHOD(int, WSAGetLastError, (), (override));
};

// Tests for MS Windows
// --------------------
TEST(HttpreadwriteIp4TestSuite, Check_Connect_And_Wait_Connection) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // WSAGetLastError
    Mock_winsock2 mock_winsock2Obj;
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_wrong_connect_retval) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // WSAGetLastError
    Mock_winsock2 mock_winsock2Obj;
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).Times(0);

    // Test the unit
    int connect_retval{0};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_connect_error) {
    // This file descriptor is assumed to be not valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // WSAGetLastError WSAEBADF = "File handle is not valid."
    Mock_winsock2 mock_winsock2Obj;
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError()).WillOnce(Return(WSAEBADF));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_select_timeout) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select() returns 0, that is timeout
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(0));
    // WSAGetLastError
    Mock_winsock2 mock_winsock2Obj;
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_select_error) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select() returns -1, that is failure
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // WSAGetLastError
    Mock_winsock2 mock_winsock2Obj;
    EXPECT_CALL(mock_winsock2Obj, WSAGetLastError())
        .WillOnce(Return(WSAEWOULDBLOCK));

    // Test the unit
    int connect_retval{-1};
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_sockoption_error) {
    if (old_code)
        ::std::cout << "  # There are no errors from socket options checked.\n";
    else
        GTEST_SKIP() << "  OPT: There are no errors from socket options "
                        "checked. This should be revised.";
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_getsockopt_error) {
    if (old_code)
        ::std::cout << "  # getsockopt() isn't used.\n";
    else
        GTEST_SKIP()
            << "  OPT: getsockopt() isn't used. This should be revised.";
}

//
// ######################################
// Tests for Unix
// ######################################
#else  // _WIN32

// Generate function to set value refered to by 3rd argument as needed for
// getsockopt(). This allows us to mock functions that pass in a pointer,
// expecting the result to be put into that location.
ACTION_P(SetArg3IntValue, value) { *static_cast<int*>(arg3) = value; }

//
TEST(HttpreadwriteIp4TestSuite, Check_Connect_And_Wait_Connection) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                               NotNull(), NotNull()))
        .WillOnce(DoAll(SetArg3IntValue(0), Return(0)));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_wrong_connect_retval) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(_, _, _, _, _)).Times(0);

    // Test the unit
    int connect_retval{0};
    errno = 0;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_connect_error) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj, select(_, _, _, _, _)).Times(0);
    // getsockopt
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(_, _, _, _, _)).Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = ETIMEDOUT;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  0);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  0);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_select_timeout) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(0));
    // getsockopt
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                               NotNull(), NotNull()))
        .Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_select_error) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select() returns -1, that is failure
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(-1));
    // getsockopt
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                               NotNull(), NotNull()))
        .Times(0);

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_sockoption_error) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select() returns 1, that means one socket is ready for writing
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt(), the error is returned with SetArg3IntValue(1)
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                               NotNull(), NotNull()))
        .WillOnce(DoAll(SetArg3IntValue(1), Return(0)));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}

TEST(HttpreadwriteIp4TestSuite,
     Check_Connect_And_Wait_Connection_getsockopt_error) {
    // This file descriptor is assumed to be valid.
    int socketfd{258};

    // Configure expected system calls.
    // select()
    Mock_sys_select mock_sys_selectObj;
    EXPECT_CALL(mock_sys_selectObj,
                select(socketfd + 1, NULL, NotNull(), NULL, NotNull()))
        .WillOnce(Return(1));
    // getsockopt() fails with returning -1
    Mock_sys_socket mock_sys_socketObj;
    EXPECT_CALL(mock_sys_socketObj, getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                                               NotNull(), NotNull()))
        .WillOnce(Return(-1));

    // Test the unit
    int connect_retval{-1};
    errno = EINPROGRESS;
    if (old_code)
        EXPECT_EQ(::Check_Connect_And_Wait_Connection(socketfd, connect_retval),
                  -1);
    else
        EXPECT_EQ(upnplib::Check_Connect_And_Wait_Connection(socketfd,
                                                             connect_retval),
                  -1);
}
#endif // _WIN32

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib_gtest_main.inc"
}

// vim: nowrap
