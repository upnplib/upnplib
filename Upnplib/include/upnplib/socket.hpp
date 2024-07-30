#ifndef UPNPLIB_SOCKET_HPP
#define UPNPLIB_SOCKET_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-07-30
/*!
 * \file
 * \brief **Socket Module:** manage properties and methods but not connections
 * of ONE network socket to handle IPv4 and IPv6 streams and datagrams.
 */

/*!
 * \brief The socket module contains all classes and free functions to manage
 * network sockets.
 * \addtogroup upnplib-socket
 *
 * \anchor socket_module
 * This module mainly consists of the CSocket class but
 * also provides free functions to manage a socket. The problem is that socket
 * handling isn't very good portable. There is different behavior on the
 * supported platforms Unix, MacOS and Microsoft Windows. The CSocket class
 * atempts to be consistent portable on all three platforms by using common
 * behavior or by emulating missing functions on a platform.
 *
 * Specification for CSocket
 * =========================
 * The class encapsulates and manages one raw socket file descriptor. The file
 * descriptor of a valid socket object cannot be changed but the object with
 * its immutable file descriptor can be moved and assigned to another socket
 * object. Copying a socket object isn't supported because having two objects
 * with the same file descriptor may be very error prone in particular with
 * multithreading. Effort has been taken to do not cache any socket information
 * outside the socket file descriptor. All socket informations are direct set
 * and get to/from the operating system with the file descriptor. The socket
 * file descriptor is always valid except on an empty socket object.
 *
 * \anchor empty_socket
 * empty socket object
 * -------------------
 * An empty socket object can be instantiated with the default constructor,
 * e.g. `CSocket sockObj;`. It is a valid object and should be destructed. When
 * moving a socket object, the left over source object is also empty. An empty
 * socket object has an `INVALID_SOCKET` defined and no valid content. It
 * throws an exception if using any of its Setter and Getter. Moving and
 * assigning it is possible. You can test for an empty socket by looking for an
 * `INVALID_SOCKET`, e.g.
 * \code
 * CSocket sockObj; // or CSocket_basic sockObj;
 * if (static_cast<SOCKET>(sockObj) != INVALID_SOCKET) {
 *     in_port_t port = sockObj.get_port(); };
 * \endcode
 *
 * protocol family
 * ---------------
 * Only protocol family `PF_INET6` and `PF_INET` is supported. Any other
 * address family throws an exception.
 *
 * socket type
 * -----------
 * Only `SOCK_STREAM` and `SOCK_DGRAM` is supported. Any other type throws an
 * exception.
 *
 * valid socket file descriptor
 * ----------------------------
 * I get this from the C standard library function:
 * `int ::%socket(address_family, socket_type, protocol)`.
 * Other arguments than address family and socket type are not accepted. For
 * the protocol argument is always the default one used that is internal hard
 * coded with argument 0.
 *
 * options SO_REUSEADDR and SO_EXCLUSIVEADDRUSE
 * --------------------------------------------
 * I don't set the option to immediately reuse an address and I always set the
 * option `SO_EXCLUSIVEADDRUSE` on Microsoft Windows. For more details of this
 * have a look at [Socket option "reuse address"](\ref overview_reuseaddr).
 *
 * References
 * ----------
 * - <!--REF:--><a href="https://www.rfc-editor.org/rfc/rfc3493">RFC 3493</a> -
 *   Basic Socket Interface Extensions for IPv6
 * - <!--REF:--><a href="https://www.rfc-editor.org/rfc/rfc3542">RFC 3542</a> -
 *   Advanced Sockets Application Program Interface (API) for IPv6
 */

#include <upnplib/sockaddr.hpp>
#include <upnplib/synclog.hpp>
/// \cond
#include <mutex>
#include <memory>

// To be portable with BSD socket error number constants I have to
// define and use these macros with appended 'P' for portable.
#ifdef _MSC_VER
#define EBADFP WSAENOTSOCK
#define ENOTCONNP WSAENOTCONN
#define EINTRP WSAEINTR
#define EFAULTP WSAEFAULT
#define ENOMEMP WSA_NOT_ENOUGH_MEMORY
#define EINVALP WSAEINVAL
#else
#define EBADFP EBADF
#define ENOTCONNP ENOTCONN
#define EINTRP EINTR
#define EFAULTP EFAULT
#define ENOMEMP ENOMEM
#define EINVALP EINVAL
#endif
/// \endcond

namespace upnplib {

/*!
 * \brief Get information from a raw network socket file descriptor
 * <!--   ========================================================= -->
 * \ingroup upnplibAPI-socket
 * \ingroup upnplib-socket
 *
 * For general information have a look at \ref socket_module.
 *
 * This class takes the resources and results as given by the platform (Unix,
 * MacOS, MS Windows). It does not perform any emulations for unification. The
 * behavior can be different on different platforms.
 *
 * An object of this class does not take ownership of the raw socket file
 * descriptor and will never close it. This is also the reason why you cannot
 * modify the socket and only have getter available (except the setter 'load()'
 * for the raw socket file descriptor itself). But it is helpful to easily get
 * information about an existing raw socket file descriptor. Closing the file
 * descriptor is in the responsibility of the caller who created the socket. If
 * you need to manage a socket you must use CSocket.
 */
class UPNPLIB_API CSocket_basic : private SSockaddr {
  public:
    // Default constructor for an empty basic socket object
    CSocket_basic();

    /*! \brief Constructor for the socket file descriptor. Before use, it must
     * be load(). */
    CSocket_basic(SOCKET a_sfd);

    /// \cond
    // I want to restrict to only move the resource.
    // No copy constructor
    CSocket_basic(const CSocket_basic&) = delete;
    // No copy assignment operator
    CSocket_basic& operator=(CSocket_basic) = delete;
    /// \endcond

    // Destructor
    virtual ~CSocket_basic();


    /*! \name Setter
     * *************
     * @{ */
    /*! \brief Load the raw socket file descriptor from the constructor into
     * the object
     * \code
     * // Usage e.g.:
     * SOCKET sfd = ::socket(PF_INET6, SOCK_STREAM);
     * {   // Scope for sockObj, sfd must longer exist than sockObj
     *     CSocket_basic sockObj(sfd);
     *     try {
     *         sockObj.load();
     *     } catch(xcp) { handle_error(); };
     *     // Use the getter from sockObj
     * }
     * ::close(sfd);
     * \endcode
     *
     * The socket file descriptor was given with the constructor. This object
     * does not take ownership of the socket file descriptor and will never
     * close it. Closing is in the responsibility of the caller who created the
     * socket. Initializing it again is possible but is only waste of
     * resources. The result is the same as before.
     *
     * \exception std::runtime_error Given socket file descriptor is invalid. */
    void load();
    /// @} Setter


    /*! \name Getter
     * *************
     * @{ */
    /*! \brief Get raw socket file descriptor.
     * \code
     * // Usage e.g.:
     * CSocket_basic sockObj(valid_socket_fd);
     * try {
     *     sockObj.load();
     * } catch(xcp) { handle_error(); };
     * SOCKET sfd = sockObj;
     * \endcode */
    operator const SOCKET&() const;

    /*! \brief Get socket [address family](\ref glossary_af) */
    sa_family_t get_family() const;

    /*! \brief Get [netaddress](\ref glossary_netaddr) without port. */
    const std::string& netaddr() override;

    /*! \brief Get [netaddress](\ref glossary_netaddr) with port. */
    const std::string& netaddrp() override;

    /*! \brief Get the [port](\ref glossary_port) number. */
    in_port_t get_port() const override;

    /*! \brief Get the [socket type](\ref glossary_socktype) `SOCK_STREAM` or
     * `SOCK_DGRAM`.
     *
     * Throws exception std::runtime_error if query option fails.
     *
     * \todo Check if SOCK_UNDEF is also possible, maybe with an empty socket */
    int get_socktype() const;

    /*! \brief Get the error that is given from the socket as option.
     *
     * This is not a system error from the operating system (with POSIX
     * returned in \b errno). It is the error that can be queried as option
     * from the socket.
     *
     * Throws exception std::runtime_error if query option fails. */
    int get_sockerr() const;

    /*! \brief Get status if reusing address is enabled.
     *
     * For details to this option have a look at
     * [option "reuse address"](\ref overview_reuseaddr).
     *
     * Throws exception std::runtime_error if query option fails. */
    bool is_reuse_addr() const;

    /*! \brief Get status if socket is bound to a local
     * \glos{netaddr,netaddress}.
     *
     * I assume that a valid socket file descriptor with unknown address (all
     * zero) and port 0 is not bound. */
    bool is_bound();
    /// @} Getter

  protected:
    /// \cond
    // This is the raw socket file descriptor
    SOCKET m_sfd{INVALID_SOCKET};

    // Mutex to protect concurrent binding a socket.
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    mutable std::mutex m_bound_mutex;
    /// \endcond

  private:
    // Hint from the constructor what socket file descriptor to use.
    const SOCKET m_sfd_hint{INVALID_SOCKET};

    // Helper method
    void m_get_addr_from_socket() const;
};


/*!
 * \brief Manage all aspects of a network socket.
 * \ingroup upnplibAPI-socket
 * \ingroup upnplib-socket
 *
 * For general information have a look at \ref socket_module.
 *********************************************** */
class UPNPLIB_API CSocket : public CSocket_basic {
  public:
    /*! \brief Default constructor for an
     * [empty socket object](\ref empty_socket) */
    CSocket();

    /// \brief Constructor for a new socket file descriptor that must be load()
    CSocket(sa_family_t a_family, /*!<  [in] PF_INET6 or PF_INET. PF_UNSPEC is
                                             not accepted */
            int a_socktype /*!<         [in] SOCK_STREAM or SOCK_DGRAM */);

    /*! \brief Move constructor
     *
     * This moves the socket object to a new instantiated socket object and
     * also transfers ownership to the new object. That means the destination
     * also manage and frees its resources now. After moving, the source object
     * is still valid but empty with an INVALID_SOCKET. Using its methods will
     * throw exceptions. But you can assign (operator=()) another socket object
     * to it again.
     * \code
     * // Usage e.g.:
     * CSocket sock1Obj(PF_INET6, SOCK_STREAM);
     * try {
     *     sock1Obj.load();
     * } catch(xcp) { // handle error }
     * CSocket sock2Obj{std::move(sock1Obj)};
     * \endcode
     * */
    CSocket(CSocket&&);

    /*! \brief Assignment operator
     *
     * <!-- With parameter as value this is used as copy- and move-assignment
     * operator. The correct usage (move) is evaluated by the compiler. Here
     * only the move constructor can be used. -->
     * This moves the socket object to another already existing socket object
     * and also transfers ownership to it. That means the destination object
     * also manage and frees its resources now. After moving, the source object
     * is still valid but empty with an INVALID_SOCKET. Using its methods will
     * throw exceptions.
     * \code
     * // Usage e.g.:
     * CSocket sock1Obj(PF_INET6, SOCK_STREAM);
     * try {
     *     sock1Obj.load();
     * } catch(xcp) { // handle error }
     * CSocket sock2Obj;
     * sock2Obj = std::move(sock1Obj);
     * \endcode */
    CSocket& operator=(CSocket);

    /// \brief Destructor
    virtual ~CSocket();

    /*! \name Setter
     * *************
     * @{ */

    /*! \brief Initialize the object with the hints given by the constructor
     * \code
     * // Usage e.g.:
     * CSocket sockObj(PF_INET6, SOCK_STREAM);
     * try {
     *     sockObj.load();
     * } catch(xcp) { handle_error(); }
     * \endcode */
    void load();

    /*! \brief Set IPV6_V6ONLY
     * - IPV6_V6ONLY = **true**: the socket is restricted to sending and
     *   receiving IPv6 packets only. In this case, an IPv4 and an IPv6
     *   application can bind to a single port at the same time.
     * - IPV6_V6ONLY = **false**: the socket can be used to send and receive
     *   packets to and from an IPv6 address or an IPv4-mapped IPv6 address.
     * - Bind a socket for PF_INET6 to a local address will always set its
     *   option IPV6_V6ONLY.
     * - On Unix platforms IPV6_V6ONLY is false by default and cannot be
     *   modified on sockets for PF_INET. Binding this to a local address
     *   results in a IPv4 socket with IPV6_V6ONLY **unset**.
     * - On Microsoft Windows IPV6_V6ONLY is true by default and cannot be
     *   modified on sockets for PF_INET. Binding it to a local address results
     *   in a IPv4 socket with IPV6_V6ONLY **set**. This does not make sense
     *   and I assume that the option is ignored by the underlaying ip stack in
     *   this case.
     * - The option IPV6_V6ONLY can never be modified on a sochet that is
     *   already bound to a local address.
     *
     * If the setter cannot fulfill the request it silently ignores it and does
     * not modify the socket. Other system errors may throw an exception (e.g.
     * using an invalid socket etc.).
     *
     * To get the current setting use CSocket::is_v6only(). */
    void set_v6only(const bool);

    /*! \brief Bind socket to a local interface address
     * \code
     * // Usage e.g.:
     * CSocket sockObj(PF_INET6, SOCK_STREAM);
     * try {
     *     sockObj.load();
     *     sockObj.bind("[::1]", "8080");
     * } catch(xcp) { // handle error }
     * \endcode
     *
     * This method uses internally the system function <a
     * href="https://www.man7.org/linux/man-pages/man3/getaddrinfo.3.html">::%getaddrinfo()</a>
     * to provide possible local socket addresses. If the AI_PASSIVE flag is
     * specified with **a_flags**, and **a_node** is empty (""), then the
     * selected local socket addresses will be suitable for **binding** a
     * socket that will **accept** connections. The selected local socket
     * address will contain the "wildcard address" (INADDR_ANY for IPv4
     * addresses, IN6ADDR_ANY_INIT for IPv6 address). The wildcard address is
     * used by applications (typically servers) that intend to accept
     * connections on any of the host's network addresses. If **a_node** is not
     * empty (""), then the AI_PASSIVE flag is ignored.
     * \code
     * // typical for server listening
     * sockObj.bind("", "54839", AI_PASSIVE);
     * \endcode
     *
     * If the AI_PASSIVE flag is not set, then the selected local socket
     * addresses will be suitable for use with **connect**, **sendto**, or
     * **sendmsg** (typically clients). If **a_node** is empty ("") and flag
     * AI_NUMERICHOST not set then you will get an exception: no address for
     * hostname "". With AI_NUMERICHOST the unknown address "[::]" or "0.0.0.0"
     * is used.
     * \code
     * // typical for client connect
     * sockObj.bind("[2001:db8::1]", "49123");
     * // or
     * sockObj.bind("", "51593"); // uses "[::1]:51593" or "127.0.0.1:51593"
     * \endcode
     *
     * With protocol family PF_INET6 I internally always set IPV6_V6ONLY to
     * true to be portable with same behavior on all platforms. This is default
     * on Unix platforms when binding the address and cannot be modified. MacOS
     * does not modify IPV6_V6ONLY with binding. On Microsoft Windows
     * IPV6_V6ONLY is already set by default.
     *
     * There is additional information at set_v6only() */
    void bind(
        /*! [in] local interface address */
        const std::string& a_node,
        /*! [in] Port of the local interface address. This is a string argument
         * to be able to use service names instead of only numbers. */
        const std::string& a_port,
        /*! [in] Optional: this field specifies additional options, as
         * described at <a
         * href="https://www.man7.org/linux/man-pages/man3/getaddrinfo.3.html">getaddrinfo(3)
         * — Linux manual page</a> or at <a
         * href="https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo">Microsoft
         * Build — getaddrinfo function</a>. Multiple flags are specified by
         * bitwise OR-ing them together. Example is: `AI_PASSIVE |
         * AI_NUMERICHOST | AI_NUMERICSERV` */
        const int a_flags = 0);

    /*! \brief Set socket to listen
     *
     * On Linux there is a socket option SO_ACCEPTCONN that can be get with
     * system function ::%getsockopt(). This option shows if the socket is set
     * to passive listen. But it is not portable. MacOS does not support it. So
     * this flag has to be managed here. Look for details at <!--REF:--><a
     * href="https://stackoverflow.com/q/75942911/5014688">How to get option on
     * MacOS if a socket is set to listen?</a> */
    void listen();
    /// @} Setter


    /*! \name Getter
     * *************
     * @{ */
    /*! \brief Get status of IPV6_V6ONLY flag
     *
     * IPV6_V6ONLY == false means allowing IPv4 and IPv6. */
    bool is_v6only() const;

    /// \brief Get status if the socket is listen to incomming network packets.
    bool is_listen() const;
    /// @} Getter

  private:
    /// \brief Protocol family to use
    const sa_family_t m_pf_hint{PF_UNSPEC};

    /// \brief Socket type to use
    const int m_socktype_hint{};

    /// \brief Mutex to protect concurrent listen a socket.
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    mutable std::mutex m_listen_mutex;
    bool m_listen{false}; // Protected by a mutex.
};


// Portable handling of socket errors
// ==================================
/*! \brief Class for portable handling of network socket errors.
 * \ingroup upnplibAPI-socket
 * \ingroup upnplib-socket
 * \code
 * // Usage e.g.:
 * CSocketErr serrObj;
 * int ret = some_function_1();
 * if (ret != 0) {
 *     serrObj.catch_error();
 *     int errid = serrObj;
 *     std::cout << "Error " << errid << ": "
 *               << serrObj.error_str() << "\n";
 * }
 * ret = some_function_2();
 * if (ret != 0) {
 *     serrObj.catch_error();
 *     std::cout << "Error " << static_cast<int>(serrObj) << ": "
 *               << serrObj.error_str() << "\n";
 * }
 * \endcode
 *
 * There is a compatibility problem with Winsock2 on the Microsoft Windows
 * platform that does not support detailed error information given in the global
 * variable 'errno' that is used by POSIX. Instead it returns them with calling
 * 'WSAGetLastError()'. This class encapsulates differences so there is no need
 * to always check the platform to get the error information.
 *
 * This class is optimized for frequent short-term use. It's a simple class
 * without inheritence and virtual methods.
 */
class UPNPLIB_API CSocketErr {
  public:
    CSocketErr();
    ~CSocketErr();
    // Delete copy constructor
    CSocketErr(const CSocketErr&) = delete;
    // Delete assignment operator
    CSocketErr& operator=(const CSocketErr&) = delete;
    /// Get error number.
    operator const int&();
    /// Catch error for later use.
    void catch_error();
    /// Get human readable error description of the catched error.
    std::string error_str() const;

  private:
    int m_errno{}; // Cached error number
};

} // namespace upnplib

#endif // UPNPLIB_SOCKET_HPP
