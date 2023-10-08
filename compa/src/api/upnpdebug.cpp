/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2023-10-08
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
// Last compare with pupnp original source file on 2023-08-19, ver 1.14.17

/*!
 * \file
 */
#define UPNP_DEBUG_C
#include <config.hpp>

#include <ithread.hpp>
#include <upnp.hpp>
#include <upnpdebug.hpp>

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <iostream>

#include <posix_overwrites.hpp>

#include <upnplib/cmake_vars.hpp>
#include <upnplib/general.hpp>

#include <umock/pthread.hpp>
#include <umock/stdio.hpp>

/*! Mutex to synchronize all the log file operations in the debug mode */
static ithread_mutex_t GlobalDebugMutex;

/*! Global log level */
static Upnp_LogLevel g_log_level = UPNP_DEFAULT_LOG_LEVEL;

/* Output file descriptor */
static FILE* filed;

/* Set if the user called setlogfilename() or setloglevel() */
static int setlogwascalled;
static int initwascalled;
/* Name of the output file. We keep a copy */
static char* fileName;

/* This is called from UpnpInit2(). So the user must call UpnpSetLogFileNames()
 * before. This can be called again, for example to rotate the log
 * file, and we try to avoid multiple calls to the mutex init, with a
 * risk of race, probably not a problem, and not worth fixing. */
int UpnpInitLog() {
    TRACE("Executing UpnpInitLog()")
    if (!initwascalled) {
        umock::pthread_h.pthread_mutex_init(&GlobalDebugMutex, NULL);
        initwascalled = 1;
    }
    /* If the user did not ask for logging do nothing */
    if (setlogwascalled == 0) {
        return UPNP_E_SUCCESS;
    }

    if (filed != nullptr && filed != stderr) {
        umock::stdio_h.fclose(filed);
        filed = nullptr;
    }

    if (fileName) {
        filed = umock::stdio_h.fopen(fileName, "a");
        if (filed == nullptr) {
            fprintf(stderr,
                    "UPnPlib Error: failed to open filename \"%s\": %s. "
                    "Fallback to stderr.\n",
                    fileName, std::strerror(errno));
        }
    }

    // If not set, always set to stderr, also as fallback for a wrong fileName.
    if (filed == nullptr)
        filed = stderr;

    return UPNP_E_SUCCESS;
}

void UpnpSetLogLevel(Upnp_LogLevel log_level) {
    TRACE("Executing UpnpSetLogLevel()")
    g_log_level = log_level;
    setlogwascalled = 1;
}

void UpnpCloseLog() {
    TRACE("Executing UpnpCloseLog()")

    /* Calling lock() assumes that someone called UpnpInitLog(), but
     * this is reasonable as it is called from UpnpInit2(). We risk a
     * crash if we do this without a lock.*/
    if (initwascalled)
        umock::pthread_h.pthread_mutex_lock(&GlobalDebugMutex);

    if (filed != nullptr && filed != stderr) {
        umock::stdio_h.fclose(filed);
    }
    filed = nullptr;
    setlogwascalled = 0;
    int init_called = initwascalled;
    initwascalled = 0;
    if (init_called) {
        umock::pthread_h.pthread_mutex_unlock(&GlobalDebugMutex);
        umock::pthread_h.pthread_mutex_destroy(&GlobalDebugMutex);
    }
}

void UpnpSetLogFileNames(const char* newFileName,
                         [[maybe_unused]] const char* ignored) {
    TRACE("Executing UpnpSetLogFileNames()")

    if (fileName) {
        free(fileName);
        fileName = nullptr;
    }
    if (newFileName && *newFileName != '\0') {
        fileName = strdup(newFileName);
    }
    setlogwascalled = 1;
}

static int DebugAtThisLevel(Upnp_LogLevel DLevel, Dbg_Module Module) {
    return (DLevel <= g_log_level) &&
           (DEBUG_ALL || (Module == SSDP && DEBUG_SSDP) ||
            (Module == SOAP && DEBUG_SOAP) || (Module == GENA && DEBUG_GENA) ||
            (Module == TPOOL && DEBUG_TPOOL) ||
            (Module == MSERV && DEBUG_MSERV) || (Module == DOM && DEBUG_DOM) ||
            (Module == HTTP && DEBUG_HTTP));
}

static void UpnpDisplayFileAndLine(FILE* a_fp, const char* DbgFileName,
                                   int DbgLineNo, Upnp_LogLevel DLevel,
                                   Dbg_Module Module) {
    char timebuf[26];
    time_t now = time(NULL);
    const char* smod;
#if 0
	char *slev;
	/* Code kept around in case, but I think it's actually more convenient
	   to display a numeric level */
	switch (DLevel) {
	case UPNP_CRITICAL: slev="CRI";break;
	case UPNP_ERROR: slev="ERR";break;
	case UPNP_INFO: slev="INF";break;
	case UPNP_ALL: slev="ALL";break;
	default: slev="UNK";break;
	}
#else
    char slev[25];
    snprintf(slev, 25, "%d", DLevel);
#endif

    switch (Module) {
    case SSDP:
        smod = "SSDP";
        break;
    case SOAP:
        smod = "SOAP";
        break;
    case GENA:
        smod = "GENA";
        break;
    case TPOOL:
        smod = "TPOL";
        break;
    case MSERV:
        smod = "MSER";
        break;
    case DOM:
        smod = "DOM_";
        break;
    case API:
        smod = "API_";
        break;
    case HTTP:
        smod = "HTTP";
        break;
    default:
        smod = "UNKN";
        break;
    }

#ifdef _WIN32
    struct tm timeinfo = {.tm_sec = 0,
                          .tm_min = 0,
                          .tm_hour = 0,
                          .tm_mday = 0,
                          .tm_mon = 0,
                          .tm_year = 0,
                          .tm_wday = 0,
                          .tm_yday = 0,
                          .tm_isdst = 0};
    localtime_s(&timeinfo, &now);
    strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", &timeinfo);
#else
    struct tm* timeinfo;
    timeinfo = localtime(&now);
    strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", timeinfo);
#endif

    fprintf(a_fp, "%s UPNP-%s-%s: Thread:0x%llX [%s:%d]: ", timebuf, smod, slev,
#ifdef __PTW32_DLLPORT
            *(unsigned long long int*)ithread_self().p
#else
            (unsigned long long int)ithread_self()
#endif
            ,
            DbgFileName, DbgLineNo);
}

void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module,
                const char* DbgFileName, int DbgLineNo, const char* FmtStr,
                ...) {
    /*fprintf(stderr, "UpnpPrintf: filed %p level %d glev %d mod %d DEBUG_ALL
      %d\n", filed, DLevel, g_log_level, Module, DEBUG_ALL);*/
    va_list ArgList;

    if (!initwascalled) {
        return;
    }

    if (!DebugAtThisLevel(DLevel, Module))
        return;
    umock::pthread_h.pthread_mutex_lock(&GlobalDebugMutex);
    if (filed == nullptr) {
        umock::pthread_h.pthread_mutex_unlock(&GlobalDebugMutex);
        return;
    }

    va_start(ArgList, FmtStr);
    if (DbgFileName) {
        // flush stdout to have a sequencial output with stderr on screen.
        fflush(stdout); // Don't mock this because we use it for debuging.
        UpnpDisplayFileAndLine(filed, DbgFileName + UPNPLIB_PROJECT_PATH_LENGTH,
                               DbgLineNo, DLevel, Module);
        vfprintf(filed, FmtStr, ArgList);
        if (filed != nullptr && filed != stderr)
            fflush(filed); // Don't mock this because we use it for debuging.
    }
    va_end(ArgList);
    umock::pthread_h.pthread_mutex_unlock(&GlobalDebugMutex);
}

/* No locking here, the app should be careful about not calling
   closelog from a separate thread... */
FILE* UpnpGetDebugFile(Upnp_LogLevel DLevel, Dbg_Module Module) {
    if (!DebugAtThisLevel(DLevel, Module)) {
        return nullptr;
    } else {
        return filed;
    }
}
