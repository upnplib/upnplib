/**************************************************************************
 *
 * Copyright (c) 2006 Rémi Turboult <r3mi@users.sourceforge.net>
 * All rights reserved.
 * Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2022-03-04
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "FreeList.hpp"
#include "upnpdebug.hpp"
#include "upnptools.hpp"

#include <iostream>
#include <sstream>
#include <string.h>

#if defined INCLUDE_DEVICE_APIS && EXCLUDE_SOAP == 0
#include "miniserver.hpp"
#endif

namespace upnplib {

void* library_info(void*) {
    // Using this will not split output string on '<<' by output from other
    // threads.
    std::stringstream msg;

    msg << "---- configuration and build -----\n";
    msg << "CMAKE_VERSION = " << CMAKE_VERSION << '\n';
    msg << "CMAKE_CXX_COMPILER = " << CMAKE_CXX_COMPILER << '\n';
    msg << "CMAKE_CXX_COMPILER_VERSION = " << CMAKE_CXX_COMPILER_VERSION
        << '\n';
    msg << "CMAKE_BUILD_TYPE = " << CMAKE_BUILD_TYPE << '\n';
    msg << "CMAKE_GENERATOR = " << CMAKE_GENERATOR << '\n';

    msg << "---- library info ----------------\n"
        << "UPNP_VERSION_STRING = " << UPNP_VERSION_STRING << "\n"
        << "UPNP_VERSION_MAJOR = " << UPNP_VERSION_MAJOR << "\n"
        << "UPNP_VERSION_MINOR = " << UPNP_VERSION_MINOR << "\n"
        << "UPNP_VERSION_PATCH = " << UPNP_VERSION_PATCH << "\n"
        << "UPNP_VERSION = " << UPNP_VERSION << "\n";
    /*
     * Check library optional features
     */
    msg << "---- user definable options ------\n";
#ifdef DEBUG
    msg << "DEBUG                 = yes\n";
#else
    msg << "DEBUG                 = no\n";
#endif

#ifdef UPNP_HAVE_DEBUG
    msg << "UPNP_HAVE_DEBUG       = yes\n";
#else
    msg << "UPNP_HAVE_DEBUG       = no\n";
#endif

#ifdef UPNP_HAVE_TOOLS
    const char* errmsg = UpnpGetErrorMessage(UPNP_E_SUCCESS);
    if (strcmp(errmsg, "UPNP_E_SUCCESS") == 0)
        msg << "UPNP_HAVE_TOOLS       = yes\n";
    else
        msg << "UPNP_HAVE_TOOLS       = yes, but does not return "
               "UPNP_E_SUCCESS\n";
#else
    msg << "UPNP_HAVE_TOOLS       = no\n";
#endif

#ifdef UPNP_HAVE_CLIENT
    msg << "UPNP_HAVE_CLIENT      = yes\n";
#else
    msg << "UPNP_HAVE_CLIENT      = no\n";
#endif

#ifdef UPNP_HAVE_DEVICE
    msg << "UPNP_HAVE_DEVICE      = yes\n";
#else
    msg << "UPNP_HAVE_DEVICE      = no\n";
#endif

#ifdef UPNP_HAVE_WEBSERVER
    msg << "UPNP_HAVE_WEBSERVER   = yes\n";
#else
    msg << "UPNP_HAVE_WEBSERVER   = no\n";
#endif

#ifdef UPNP_HAVE_SSDP
    msg << "UPNP_HAVE_SSDP        = yes\n";
#else
    msg << "UPNP_HAVE_SSDP        = no\n";
#endif

#ifdef UPNP_HAVE_OPTSSDP
    msg << "UPNP_HAVE_OPTSSDP     = yes\n";
#else
    msg << "UPNP_HAVE_OPTSSDP     = no\n";
#endif

    msg << "---- internal settings -----------\n";

#ifdef UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS
    msg << "UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS   = yes\n";
#else
    msg << "UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS   = no\n";
#endif

#ifdef INTERNAL_WEB_SERVER
    msg << "INTERNAL_WEB_SERVER                 = yes\n";
#else
    msg << "INTERNAL_WEB_SERVER                 = no\n";
#endif

    msg << "----------------------------------\n";
    std::cout << msg.str();

    return (void*)0; // calls pthread_exit()
}

// Simple check of module FreeList
// -------------------------------
#ifdef _WIN32
// There is a problem with EXPORT_SPEC. We have to modify the original source
// upnp/src/threadutil/FreeList.hpp to export its function names. Modifications
// of the original sources are not intended at this stage of re-engeneering.
void* check_freelist(void*) {
    std::cout << "-> Check module FreeList is temporary disabled on MS Windows "
                 "due to re-engeneering issues.\n";
    return (void*)0;
}
#else
// Running in the production environment here. For extended testing look at
// the Unit Tests.
void* check_freelist(void*) {
    FreeList free_list{};
    // Init with chunk size of 1 byte and max. 3 entries in the freelist.
    ::FreeListInit(&free_list, 1, 3);

    // Get new nodes. Because the freelist is empty, they should be allocated
    // from memory.
    FreeListNode* newnode1 = (FreeListNode*)::FreeListAlloc(&free_list);
    FreeListNode* newnode2 = (FreeListNode*)::FreeListAlloc(&free_list);
    FreeListNode* newnode3 = (FreeListNode*)::FreeListAlloc(&free_list);
    FreeListNode* newnode4 = (FreeListNode*)::FreeListAlloc(&free_list);

    // Put unused nodes to the freelist.
    ::FreeListFree(&free_list, newnode1);
    ::FreeListFree(&free_list, newnode2);
    ::FreeListFree(&free_list, newnode3);
    ::FreeListFree(&free_list, newnode4);

    // Get them again. They should now mostly come from the freelist without
    // expensive allocation from memory.
    newnode1 = (FreeListNode*)::FreeListAlloc(&free_list);
    newnode2 = (FreeListNode*)::FreeListAlloc(&free_list);
    newnode3 = (FreeListNode*)::FreeListAlloc(&free_list);
    newnode4 = (FreeListNode*)::FreeListAlloc(&free_list);

    // Destray the freelist. This should free all allocated memory for the
    // nodes.
    ::FreeListDestroy(&free_list);

    return (void*)0;
}
#endif // _WIN32

//
// Simple check of module upnpdebug
// --------------------------------
#ifdef _WIN32
// There is a problem with EXPORT_SPEC. We have to modify the original source
// upnp/inc/upnpdebug.hpp to export its function names. Modifications
// of the original sources are not intended at this stage of re-engeneering.
void* check_upnpdebug(void*) {
    std::cout
        << "-> Check module upnpdebug is temporary disabled on MS Windows "
           "due to re-engeneering issues.\n";
    return (void*)0;
}
#else // _WIN32

// Running in the production environment here. For extended testing look at
// the Unit Tests.
void* check_upnpdebug(void*) {
#ifdef NDEBUG
    std::cout << "-> Module upnpdebug is disabled because NDEBUG is set.\n";
#else // NDEBUG
    // This must be set before initialization
    ::UpnpSetLogLevel(UPNP_ALL);

    int rc = ::UpnpInitLog();
    if (rc != UPNP_E_SUCCESS) {
        std::cerr << "Error: UpnpInitLog() failed with return code = " << rc
                  << std::endl;
        return (void*)1;
    }
    FILE* fd = ::UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL);
    if (fd != stderr) {
        std::cerr << "Error! UpnpGetDebugFile(..) does not return \'stderr\' "
                     "for output.\n";
        return (void*)2;
    }
    ::UpnpPrintf(UPNP_ALL, API, __FILE__, __LINE__, "Simple check for %s.\n",
                 "UpnpPrintf");
    ::UpnpCloseLog();

#endif // NDEBUG
    return (void*)0;
}
#endif // _WIN32

} // namespace upnplib

//
// main entry
// ----------
int main() {
    int rc;
    void* retval;

    // Starting POSIX Threads
    // ----------------------
    std::cout << "-- Starting POSIX Threads ..." << std::endl;

    pthread_t thread_info;

    rc = pthread_create(&thread_info, NULL, &upnplib::library_info, NULL);
    if (rc != 0) {
        std::cerr << "Error! unable to create thread_info, " << rc << std::endl;
        exit(1);
    }

    pthread_t thread_freelist;

    rc = pthread_create(&thread_freelist, NULL, &upnplib::check_freelist, NULL);
    if (rc != 0) {
        std::cerr << "Error! unable to create thread_freelist, " << rc
                  << std::endl;
        exit(1);
    }

    pthread_t thread_upnpdebug;

    rc = pthread_create(&thread_upnpdebug, NULL, &upnplib::check_upnpdebug,
                        NULL);
    if (rc != 0) {
        std::cerr << "Error! unable to create thread_upnpdebug, " << rc
                  << std::endl;
        exit(1);
    }

    // Waiting for threads to finish
    // -----------------------------
    rc = pthread_join(thread_info, &retval);
    if (rc != 0) {
        std::cerr << "Error! Unable to join thread_info with rc=" << rc
                  << std::endl;
        exit(1);
    }
    if (retval != NULL) {
        std::cerr << "Error! Thread_info failed with retval="
                  << ((int)(intptr_t)retval) << std::endl;
        exit((int)(intptr_t)retval);
    }
    std::cout << "-- POSIX Thread for library info done." << std::endl;

    rc = pthread_join(thread_freelist, &retval);
    if (rc != 0) {
        std::cerr << "Error! Unable to join thread_freelist with rc=" << rc
                  << std::endl;
        exit(1);
    }
    if (retval != NULL) {
        std::cerr << "Error! Thread_info failed with retval="
                  << ((int)(intptr_t)retval) << std::endl;
        exit((int)(intptr_t)retval);
    }
    std::cout << "-- POSIX Thread to check module FreeList done." << std::endl;

    rc = pthread_join(thread_upnpdebug, &retval);
    if (rc != 0) {
        std::cerr << "Error! Unable to join thread_upnpdebug with rc=" << rc
                  << std::endl;
        exit(1);
    }
    if (retval != NULL) {
        std::cerr << "Error! Thread_upnpdebug failed with retval="
                  << ((int)(intptr_t)retval) << std::endl;
        exit((int)(intptr_t)retval);
    }
    std::cout << "-- POSIX Thread to check module upnpdebug done." << std::endl;

    return 0;
}
