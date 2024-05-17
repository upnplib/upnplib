#ifndef UPNPLIB_INCLUDE_ADDRINFO_HPP
#define UPNPLIB_INCLUDE_ADDRINFO_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-17
/*!
 * \file
 * \brief Declaration of the Addrinfo class.
 */

#include <upnplib/sockaddr.hpp>

namespace upnplib {

/*!
 * \brief Wrap C style ::%addrinfo() structure with a class
 * <!--   ================================================= -->
 * \ingroup upnplib-addrmodul
 *
 * We need a copy constructor and a copy assignment operator. For details see
 * more at <!--REF:--> <a
 * href="https://stackoverflow.com/q/4172722/5014688">What is The Rule of
 * Three?</a>
 */
class UPNPLIB_API CAddrinfo : public SSockaddr {
  public:
    /*! \brief Constructor for getting an address information with port number
     * string. */
    CAddrinfo(const std::string& a_node, const std::string& a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    /*! \brief Constructor for getting an address information with numeric port
     * number. */
    CAddrinfo(const std::string& a_node, in_port_t a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    /*! \brief Copy constructor
     * \code
     * ~$ // Usage e.g.:
     * ~$ CAddrinfo ai2 = ai1; // ai1 is an instantiated valid object
     * ~$ // or
     * ~$ CAddrinfo ai2{ai1};
     * \endcode
     * \exception ... Not expected */
    CAddrinfo(const CAddrinfo& that);

    /*! \brief copy assignment operator
     * \code
     * ~$ // Usage e.g.:
     * ~$ ai2 = ai1; // ai1 and ai2 are instantiated valid objects.
     * \endcode */
    CAddrinfo& operator=(CAddrinfo that) noexcept;

    /*! \brief Destructor */
    virtual ~CAddrinfo();


    /*! \name Setter
     * *************
     * @{ */
    /*! \brief Initialize address information that is given by syscall
     * ::%getaddrinfo()
     *
     * \code
~$ // Usage e.g.:
~$ CAddrinfo ai("[2001:db8::1]", 50050, AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
~$ try {
~$     ai.init();
~$ } catch (const std::runtime_error& e) { handle_failed_address_info();
~$ } catch (const std::invalid_argument& e) { handle_other_error();
~$ }
~$ normal_execution();
     * \endcode
     *
     * Usually this setter is called one time after constructing the object.
     * This gets an address information from the operating system that may also
     * use its internal name resolver inclusive contacting external DNS server.
     * If you use the flag **AI_NUMERICHOST** with the constructor then a
     * possible expensive name resolution to DNS server is suppressed.
     *
     * Because always the same cached hints given with the constructor are used
     * we also get the same information but with new allocated memory. So it
     * doesn't make much sense to call it more than one time, but doesn't hurt
     * except waste of resources. But calling init() again is very important
     * for internal use within the copy constructor.
     *
     * \exception std::runtime_error Failed to get address information, node or
     * service not known. Maybe an alphanumeric node name that cannot be
     * resolved. Or the DNS server is temporary not available.
     * \exception std::invalid_argument Other system error, e.g. address family
     * or socket type not supported, etc.
     *
     * Provides strong exception guarantee. It should be noted that are
     * different error messages returned by different platforms.
     */
    void init();
    /// @} Setter


    /*! \name Getter
     * *************
     * @{ */
    /*! \brief Compare operator to test if another address info is equal to
     * this.
     *
     * It only supports AF_INET6 and AF_INET. For all other address families it
     * returns false.
     *
     * \returns
     *  \b true if addresses are logical equal\n
     *  \b false otherwise */
    bool operator==(const CAddrinfo&) const noexcept;


    /*! \brief Read access to members of the addrinfo structure
     * \code
     * ~$ // Usage e.g.:
     * ~$ CAddrinfo ai("localhost", 50001, AF_UNSPEC, SOCK_STREAM);
     * ~$ try {
     * ~$     ai.init();
     * ~$ } catch (xcp) { handle_error(); }
     * ~$ if(ai->ai_socktype == SOCK_DGRAM) {} // is SOCK_STREAM;
     * ~$ if(ai->ai_family == AF_INET6) { do_this(); };
     * \endcode */
    // REF:_<a_href="https://stackoverflow.com/a/8782794/5014688">Overloading_member_access_operators_->,_.*</a>
    ::addrinfo* operator->() const noexcept;
    /// @} Getter


  private:
    // Cache the hints that are given by the user, so we can always get
    // identical address information from the operating system.
    DISABLE_MSVC_WARN_4251
    std::string m_node;
    std::string m_service;
    ENABLE_MSVC_WARN
    mutable addrinfo m_hints;

    // This pointer is the reason why we need a copy constructor and a copy
    // assignment operator.
    addrinfo* m_res{&m_hints};

    // An empty C string that I can point to for initialization so there is no
    // nullptr with potential segfault.
    char m_empty_c_str[1]{""};

    // Private method to free allocated memory for address information.
    void free_addrinfo() noexcept;
};

} // namespace upnplib

#endif // UPNPLIB_INCLUDE_ADDRINFO_HPP
