#ifndef UPNPLIB_SOCKET_HPP
#define UPNPLIB_SOCKET_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-03

// Helpful link for ip address structures:
// REF: [sockaddr structures as union]
// (https://stackoverflow.com/a/76548581/5014688)


// Socket module
// =============
// This module mainly consists of the CSocket class but also provides free
// functions to manage a socket. The problem is that socket handling isn't very
// good portable. There is different behavior on the supported platforms Unix,
// MacOS and Microsoft Windows. The CSocket class atempts to be consistent
// portable on all three platforms by using common behavior or by emulating
// missing functions on a platform.
//
// Specification for CSocket
// -------------------------
// The class encapsulates and manages one raw socket file descriptor. The file
// descriptor of a valid socket object cannot be changed but the object with
// its immutable file descriptor can be moved and assigned to another socket
// object. Copying a socket object isn't supported because having two objects
// with the same file descriptor may be very error prone in particular with
// multithreading. Effort has been taken to do not cache any socket information
// outside the socket file descriptor. All socket informations are direct set
// and get to/from the operating system with the file descriptor. The socket
// file descriptor is always valid except on an empty socket object.
//
// --- empty socket object ---
// An empty socket object can be instantiated with the default constructor,
// e.g. 'CSocket sockObj;'. It is a valid object and should be destructed. When
// moving a socket object, the left over source object is also empty. An empty
// socket object has an INVALID_SOCKET defined and no valid content. It throws
// an exception if using any of its Setter and Getter. Moving and assigning it
// is possible.
//
// --- address family ---
// Only address family AF_INET6 and AF_INET is supported. Any other address
// family throws an exception.
//
// --- socket type ---
// Only SOCK_STREAM and SOCK_DGRAM is supported. Any other type throws an
// exception.
//
// --- valid socket file descriptor ---
// I get this from the C standard library function
// int ::socket(address_family, socket_type, protocol).
// Other arguments than address family and socket type do not instantiate a
// valid socket object and throw an exception. For the protocol argument is
// always the default one used that is internal hard coded with argument 0.
//
// --- option SO_REUSEADDR ---
// I don't set the option to immediately reuse an address of a local listening
// socket after it was closed. Instead I respect the 'TIME_WAIT'. This is a
// security issue as described at
// REF: [Bind: Address Already in Use]
// (https://hea-www.harvard.edu/~fine/Tech/addrinuse.html).
// I reset SO_REUSEADDR with constructing a socket object on all platforms if
// it should be set by default. This is unclear on WIN32. See next note.
//
// --- option SO_EXCLUSIVEADDRUSE on Microsoft Windows ---
// THIS IS AN IMPORTANT SECURITY ISSUE! Lock at
// REF: [Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE]
// (https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse#application-strategies)
// I always set this option with constructing a socket object on a WIN32
// platform.
// --Ingo


#include <upnplib/visibility.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/sockaddr.hpp>
#include <upnplib/addrinfo.hpp>
#include <string>
#include <mutex>

// To be portable with BSD socket error number constants I have to
// define and use these macros with appended 'P' for portable.
#ifdef _MSC_VER
#define EBADFP WSAENOTSOCK
#define ENOTCONNP WSAENOTCONN
#define EINTRP WSAEINTR
#define EFAULTP WSAEFAULT
#define ENOMEMP WSA_NOT_ENOUGH_MEMORY
#else
#define EBADFP EBADF
#define ENOTCONNP ENOTCONN
#define EINTRP EINTR
#define EFAULTP EFAULT
#define ENOMEMP ENOMEM
#endif

namespace upnplib {

// Free function to get socket type string (eg. "SOCK_STREAM") from value
// ----------------------------------------------------------------------
UPNPLIB_API std::string to_socktype_str(const int socktype);


// CSocket_basic class
// ===================
// This class takes the resources and results as given by the platform. It does
// not perform any emulations for unification. The behavior can be different on
// different platforms.
class UPNPLIB_API CSocket_basic : private SSockaddr {
  protected:
    // Default constructor for an empty socket object
    CSocket_basic();

  public:
    // Constructor with given file desciptor
    // This instantiate a socket object from a raw socket file descriptor. It
    // throws an exception if the raw socket argument is invalid. It does not
    // close the socket. This is in the response of the user who created the
    // socket.
    // Throws exception std::runtime_error.
    CSocket_basic(SOCKET);

    // Copy constructor
    // not generated by default with custom move constructor. I want to
    // restrict to only move the resource. Don't enable this.
    // CSocket(const CSocket&);

    // Destructor
    virtual ~CSocket_basic();

    // Get raw socket file descriptor, e.g.:
    // CSocket_basic sockObj; SOCKET sfd = sockObj;
    operator const SOCKET&() const;

    // Getter
    // ------
    sa_family_t get_family() const;

    const std::string& get_addr_str() override;

    const std::string& get_addrp_str() override;

    in_port_t get_port() const override;

    // Get the socket connection type, e.g. SOCK_STREAM or SOCK_DGRAM, but also
    // others. Throws exception std::runtime_error.
    int get_socktype() const;

    int get_sockerr() const;

    bool is_reuse_addr() const;

    // I assume that a valid socket file descriptor with unknown address (all
    // zero) and port 0 is not bound.
    // Throws exception std::runtime_error.
    bool is_bound();

  protected:
    // This is the raw socket file descriptor
    SOCKET m_sfd{INVALID_SOCKET};

    // Mutex to protect concurrent binding a socket.
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    mutable std::mutex m_bound_mutex;

  private:
    // Helper method
    void m_get_addr_from_socket(int line) const;
};


// CSocket class
// =============
class UPNPLIB_API CSocket : public CSocket_basic {
  public:
    // Default constructor for an empty socket object
    CSocket();

    // Constructor for new socket file descriptor
    CSocket(sa_family_t a_family, int a_socktype);

    // Move constructor
    CSocket(CSocket&&);

    // Assignment operator
    // With parameter as value this is used as copy- and move-assignment
    // operator. The correct usage (move) is evaluated by the compiler. Here
    // only the move constructor can be used (there is no copy constructor) to
    // move the parameter to the function body.
    CSocket& operator=(CSocket);

    // Destructor
    virtual ~CSocket();

    // Setter
    // ------
    // Set socket to bind.
    // I use a string argument for a_port to be able to use service names
    // instead of only numbers.
    void bind(const std::string& a_node, const std::string& a_port,
              const int a_flags = 0);

    // Set socket to listen.
    // On Linux there is a socket option SO_ACCEPTCONN that can be get with
    // system function ::getsockopt(). This option shows if the socket is set to
    // passive listen. But it is not portable. MacOS does not support it. So
    // this flag has to be managed here. Look for details at
    // REF:_[How_to_get_option_on_MacOS_if_a_socket_is_set_to_listen?](https://stackoverflow.com/q/75942911/5014688)
    void listen();

    // Set IPV6_V6ONLY
    // * This flag can only be set on sockets of address family AF_INET6.
    // * It is always false on a socket with address family AF_INET.
    // * It is always true on Unix platforms after binding a socket to an
    //   address of family AF_INET6 if passive mode isn't set on the address
    //   info (flag AI_PASSIVE).
    // * With an address info set to passive listen on local addresses (flag
    //   AI_PASSIVE) IPV6_V6ONLY can be modified before binding it to an
    //   address. After bind it hasn't changed. This means the socket can
    //   listen to IPv6 and IPv4 connections if IPV6_V6ONLY is set to false.
    // * It can never be modified on a sochet that is bound to an address.
    //
    // If one of the conditions above doesn't match, the setter silently
    // ignores the request and will not modify the socket. Other system errors
    // may throw an exception (e.g. using an invalid socket etc.).
    //
    // To get the current setting use CSocket::is_v6only().
    void set_v6only(const bool);


    // Getter
    // ------
    // IPV6_V6ONLY = false means allowing IPv4 and IPv6.
    bool is_v6only() const;

    bool is_listen() const;

  private:
    // Mutex to protect concurrent listen a socket.
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    mutable std::mutex m_listen_mutex;
    bool m_listen{false}; // Protected by a mutex.
};


// Initialize and cleanup Microsoft Windows Sockets
// ================================================
// Winsock needs to be initialized before using it and it needs to be freed. I
// do that with a class, following the RAII paradigm. Multiple initialization
// doesn't matter. This is managed by the operating system with a counter. It
// ensures that winsock is initialzed only one time and freed with the last
// free call. --Ingo

#ifdef _MSC_VER
class UPNPLIB_API CWSAStartup {
  public:
    CWSAStartup();
    virtual ~CWSAStartup();
};

#define WINSOCK_INIT upnplib::CWSAStartup winsock_init;
#else
#define WINSOCK_INIT
#endif // _MSC_VER


// Portable handling of socket errors
// ==================================
// There is a problem that Winsock2 on the Microsoft Windows platform does not
// support detailed error information given in the global variable 'errno'.
// Instead it returns them with calling 'WSAGetLastError()'. This class
// encapsulates this difference so there is no need to always check the platform
// to get the error information.
class UPNPLIB_API CSocketError {
  public:
    CSocketError();
    ~CSocketError();

    // Get error number, e.g.:
    // CSocketError sockerrObj; int sock_err = sockerrObj;
    operator const int&() const;

    // Setter
    void catch_error();

    // Getter
    std::string get_error_str();

  private:
    int m_errno{}; // Cached error number
};

} // namespace upnplib

#endif // UPNPLIB_SOCKET_HPP
