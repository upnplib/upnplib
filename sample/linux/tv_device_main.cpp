/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2022-02-21
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * - Neither name of Intel Corporation nor the names of its contributors
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
 ******************************************************************************/

#include "tv_device.hpp"
#include <signal.h>

int main(int argc, char* argv[]) {
    int rc;
    pthread_t cmdloop_thread;
    int code;
#ifndef _WIN32
    int sig;
    sigset_t sigs_to_catch;
#endif

    rc = device_main(argc, argv);
    if (rc != UPNP_E_SUCCESS) {
        return rc;
    }

    /* start a command loop thread */
    code = pthread_create(&cmdloop_thread, NULL, TvDeviceCommandLoop, NULL);
    if (code != 0) {
        return UPNP_E_INTERNAL_ERROR;
    }
#ifdef _WIN32
    pthread_join(cmdloop_thread, NULL);
#else
    /* Catch Ctrl-C and properly shutdown */
    sigemptyset(&sigs_to_catch);
    /* Comment the line below if you are running gdb, otherwise you will be
     * unable to interrupt with CTRL-C. */
    sigaddset(&sigs_to_catch, SIGINT);
    sigwait(&sigs_to_catch, &sig);
    SampleUtil_Print("Shutting down on signal %d...\n", sig);
#endif
    rc = TvDeviceStop();

    return rc;
}
