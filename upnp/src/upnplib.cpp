/**************************************************************************
 *
 * Copyright (c) 2006 Rémi Turboult <r3mi@users.sourceforge.net>
 * All rights reserved.
 *
 * Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2021-08-17
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

#include "pthread.h"
#include "upnpconfig.h"
#include <iostream>

namespace // no name, i.e. anonymous for file scope
          // this is the C++ way for decorator STATIC
{
void* start_routine(void*) {
    std::cout << "-- Thread started" << std::endl;

    std::cout << "UPNP_VERSION_STRING = " << UPNP_VERSION_STRING << "\n";
    std::cout << "UPNP_VERSION_MAJOR = " << UPNP_VERSION_MAJOR << "\n";
    std::cout << "UPNP_VERSION_MINOR = " << UPNP_VERSION_MINOR << "\n";
    std::cout << "UPNP_VERSION_PATCH = " << UPNP_VERSION_PATCH << "\n";
    std::cout << "UPNP_VERSION = " << UPNP_VERSION << "\n";

/*
 * Check library optional features
 */
#if UPNP_HAVE_DEBUG
    std::cout << "UPNP_HAVE_DEBUG \t= yes\n";
#else
    std::cout << "UPNP_HAVE_DEBUG \t= no\n";
#endif

#if UPNP_HAVE_CLIENT
    std::cout << "UPNP_HAVE_CLIENT \t= yes\n";
#else
    std::cout << "UPNP_HAVE_CLIENT \t= no\n";
#endif

#if UPNP_HAVE_DEVICE
    std::cout << "UPNP_HAVE_DEVICE \t= yes\n";
#else
    std::cout << "UPNP_HAVE_DEVICE \t= no\n";
#endif

#if UPNP_HAVE_WEBSERVER
    std::cout << "UPNP_HAVE_WEBSERVER \t= yes\n";
#else
    std::cout << "UPNP_HAVE_WEBSERVER \t= no\n";
#endif

#if UPNP_HAVE_TOOLS
    std::cout << "UPNP_HAVE_TOOLS \t= yes\n";
#else
    std::cout << "UPNP_HAVE_TOOLS \t= no\n";
#endif

    std::cout << "-- Thread ended" << std::endl;
    return (0); // calls pthread_exit()
}
} // namespace

int main() {
    pthread_t thread;
    int rc;

    rc = pthread_create(&thread, NULL, &start_routine, NULL);
    if (rc) {
        std::cerr << "Error! unable to create thread, " << rc << std::endl;
        exit(-1);
    }

    pthread_exit(NULL); // last thread in process: exits program with status 0
}
