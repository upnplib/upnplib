#ifndef UPNPLIB_SOCKET_CLASS_HPP
#define UPNPLIB_SOCKET_CLASS_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-05-05

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <upnplib/visibility.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/addrinfo.hpp>
#include <upnplib/sockaddr.hpp>
#include <mutex>

namespace upnplib {

// Initialize and cleanup Microsoft Windows Sockets portable
// ---------------------------------------------------------
#ifdef _MSC_VER
class UPNPLIB_API CWSAStartup {
  public:
    CWSAStartup();
    virtual ~CWSAStartup();
};
// Winsock Init Portable
#define WINSOCK_INIT_P upnplib::CWSAStartup winsock_init;
#else
#define WINSOCK_INIT_P
#endif


// Wrap socket() system call
// -------------------------
// To copy a socket doesn't make sense. So this class only supports moving a
// socket. After moving, the moved-from object is still valid but contains an
// INVALID_SOCKET. It can be successful destructed.
// An instantiation of this class without arguments will give an empty object.
// You can move another usable CSocket to it.
// REF:_[What_is_move_semantics?](https://stackoverflow.com/q/3106110/5014688)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
// REF: 'type' : class 'type1' needs to have dll-interface to be used by clients
// of class 'type2'. C4251 can be ignored if your class is derived from a type
// in the C++ Standard Library.
// https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
#endif

class UPNPLIB_API CSocket {
  public:
    // Default constructor for an empty socket object
    CSocket();

    // Constructor for new socket file descriptor
    CSocket(sa_family_t a_domain, int a_type, int a_protocol = 0);

    // Constructor with given file desciptor
    // This instantiate a socket object from a raw socket file descriptor. It
    // throws an exception if the raw socket argument is already in use or
    // invalid.
    CSocket(SOCKET);

    // Copy constructor
    // not generated by default with custom move member functions. We want to
    // restrict to only move the resource. Don't enable it.
    // CSocket(const CSocket&);

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

    // Get raw socket file descriptor, e.g.: CSocket sock; SOCKET sfd = sock;
    operator SOCKET&() const;

    // Setter: set socket to bind.
    // Binding a socket address (given with CAddrinfo) with a different socket
    // type (e.g. SOCK_STREAM, SOCK_DGRAM, etc.) than that of the socket is not
    // supported and throw an error.
    void bind(const CAddrinfo& a_addrObj);

    // Setter: set socket to listen.
    // On Linux there is a socket option SO_ACCEPTCONN that can be get with
    // system function ::getsockopt(). This option shows if the socket is set to
    // passive listen. But it is not portable. MacOS does not support it. So
    // this flag has to be managed here. Look for details at
    // REF:_[How_to_get_option_on_MacOS_if_a_socket_is_set_to_listen?](https://stackoverflow.com/q/75942911/5014688)
    void listen();

    // Getter
    std::string get_addr_str() const;
    // Exception: std::invalid_argument if the socket isn't bound.

    uint16_t get_port() const;
    // get_port() throws an error if the socket is not bound.

    sa_family_t get_af() const;
    // Exception: std::runtime_error if using a bad file descriptor

    int get_sockerr() const;
    bool is_reuse_addr() const;
    bool is_v6only() const;
    bool is_bind() const;
    bool is_listen() const;

  private:
    SOCKET m_sfd{INVALID_SOCKET};

    // We could use getsockopt() with SO_DOMAIN to get the address family
    // direct from the socket. But this is only supported on Linux platforms,
    // not on MacOS. There is no option SO_DOMAIN given.
    // REF:_[iOS_Manual_Pages_getsockopt](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/getsockopt.2.html)
    // To be portable it is needed to cache it.
    sa_family_t m_af{AF_UNSPEC}; // used address family e.g. AF_INET6

    // Cache if other system functions where called.
    mutable std::mutex m_bound_mutex;
    bool m_bound{false}; // Protected by a mutex.
    mutable std::mutex m_listen_mutex;
    bool m_listen{false}; // Protected by a mutex.

    UPNPLIB_LOCAL int get_sockopt_int(int a_level, int a_optname,
                                      const std::string& a_optname_str) const;

    UPNPLIB_LOCAL void get_sockname(sockaddr_storage*) const;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace upnplib

#endif // UPNPLIB_SOCKET_CLASS_HPP
