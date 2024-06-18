// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-20
/*!
 * \file
 * \brief Global used flags, classes and emulated system functions.
 *
 * At least one of the global used constants or variables here is used nearly
 * by all compile units. This ensures that this unit is always linked to the
 * library no matter what options are selected. So I use it also for an
 * authomatic initialization of the library with no need to call an init
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

namespace upnplib {

// SUPPRESS_MSVC_WARN_4273_NEXT_LINE // don't do that
UPNPLIB_API bool g_dbug{false};

/*!
 * \brief Initialize and cleanup Microsoft Windows Sockets
 * <!--   ================================================ -->
 * \ingroup upnplib-socket
 *
 * Winsock needs to be initialized before using it and it needs to be freed. I
 * do that with a class, following the RAII paradigm. Multiple initialization
 * doesn't matter. This is managed by the operating system with a counter. It
 * ensures that winsock is initialzed only one time and freed with the last
 * free call. This is only done on Microsoft Windows.
 */
#ifdef _MSC_VER
class CWSAStartup {
  public:
    CWSAStartup() {
        // TRACE2(this, " Construct CWSAStartup")
        // Due to MSVC_WARN_4273, I will not use TRACE() with this global
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
            // from other threads.
            std::string msg{"UPnPlib [" + std::string(__FUNCTION__) +
                            "] CRITICAL MSG1003: Failed to initialize Windows "
                            "sockets, WSAStartup() returns (" +
                            std::to_string(rc) + ") \"" +
                            std::system_category().message(rc) + "\"\n"};
            std::cerr << msg;
        }
    }

    // No copy constructor
    CWSAStartup(const CWSAStartup&) = delete;
    // No copy assignment operator
    CWSAStartup& operator=(CWSAStartup) = delete;

    virtual ~CWSAStartup() {
        // TRACE2(this, " Destruct CWSAStartup")
        // Due to MSVC_WARN_4273, I will not use TRACE() with this global
        // linkage
#ifdef UPNPLIB_WITH_TRACE
        std::cout << "TRACE[Upnplib/src/global.cpp:" << __LINE__ << "] " << this
                  << " Destruct CWSAStartup"
                  << "\n";
#endif
        ::WSACleanup();
    }
};

// This initialize and cleanup the Microsoft Windows Socket subsystem
const CWSAStartup init_winsock;
#endif // _MSC_VER

} // namespace upnplib

/// \endcond
