// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-21
/*!
 * \file
 * \brief Global used flags, classes and emulated system functions.
 *
 * At least one of the global used constants or variables in this file is used
 * nearly by all compile units. This ensures that this unit is always linked to
 * the library no matter what options are selected. So I use it also for an
 * automatic initialization of the library with no need to call an init
 * function by the user.
 */

#include <upnplib/global.ipp>
#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
/// \cond
#include <iostream>

// strndup() is a GNU extension.
#ifndef HAVE_STRNDUP
char* strndup(const char* __string, size_t __n) {
    size_t strsize = strnlen(__string, __n);
    char* newstr = (char*)malloc(strsize + 1);
    if (newstr == NULL)
        return NULL;

    strncpy(newstr, __string, strsize);
    newstr[strsize] = 0;

    return newstr;
}
#endif
/// \endcond

namespace upnplib {

// SUPPRESS_MSVC_WARN_4273_NEXT_LINE // don't do that
UPNPLIB_API bool g_dbug{false};


namespace {

/*!
 * \brief Initialize and cleanup Microsoft Windows Sockets
 * <!--   ================================================ -->
 * \ingroup upnplib-socket
 * \note This class is not available for the user and only used on Microsoft
 * Windows.
 * \copydetails global.cpp
 *
 * Winsock needs to be initialized before using it and it needs to be freed. I
 * do that with a class, following the RAII paradigm. The initialization is
 * automatically done by the constructor when using the library. The class is
 * not available for the user. Multiple initialization doesn't matter. This is
 * managed by the operating system with a counter. It ensures that winsock is
 * initialzed only one time and freed with the last free call.
 */
#if defined(_MSC_VER) || defined(DOXYGEN_RUN)
class CWSAStartup {
  public:
    CWSAStartup() {
        // TRACE2(this, " Construct CWSAStartup")
        // Due to MSVC_WARN_4273, I will not use TRACE2() with this global
        // linkage
#ifdef UPNPLIB_WITH_TRACE
        std::cout << "TRACE[Upnplib/src/global.cpp:" << __LINE__ << "] " << this
                  << " Construct CWSAStartup"
                  << "\n";
#endif
        WSADATA wsaData;
        int rc = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc != 0) {
            // Prepare output string will not split its output on '<<' by output
            // from other threads. This important messaage should be unsplitted.
            std::string msg{"UPnPlib [" + std::string(__FUNCTION__) +
                            "] CRITICAL MSG1003: Failed to initialize Windows "
                            "sockets, WSAStartup() returns (" +
                            std::to_string(rc) + ") \"" +
                            std::system_category().message(rc) + "\"\n"};
            std::cerr << msg;
        }
    }

    /// \cond
    // No copy constructor
    CWSAStartup(const CWSAStartup&) = delete;
    // No copy assignment operator
    CWSAStartup& operator=(CWSAStartup) = delete;
    /// \endcond

    virtual ~CWSAStartup() {
        // TRACE2(this, " Destruct CWSAStartup")
        // Due to MSVC_WARN_4273, I will not use TRACE2() with this global
        // linkage
#ifdef UPNPLIB_WITH_TRACE
        std::cout << "TRACE[Upnplib/src/global.cpp:" << __LINE__ << "] " << this
                  << " Destruct CWSAStartup"
                  << "\n";
#endif
        ::WSACleanup();
    }
};

/*! \brief This object initialize and cleanup the Microsoft Windows Socket
 * subsystem
 * \ingroup upnplib-socket */
const CWSAStartup init_winsock;
#endif // _MSC_VER

} // anonymous namespace

} // namespace upnplib
