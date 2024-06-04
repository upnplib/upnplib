#ifndef _MSC_VER

// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-04
/*!
 * \file
 * \brief Definition of common used classes and free functions for network
 * connections.
 */

#include <upnplib/connection_common.hpp>
#include <upnplib/synclog.hpp>

/// \cond
namespace upnplib {

CSigpipe_scoped::CSigpipe_scoped() {
    TRACE("Construct CSigpipe_scoped");
    /* We want to ignore possible SIGPIPE that we can generate on write. SIGPIPE
     * is delivered *synchronously* and *only* to the thread doing the write. So
     * if it is reported as already pending (which means the thread blocks it),
     * then we do nothing: if we generate SIGPIPE, it will be merged with the
     * pending one (there's no queuing), and that suits us well. If it is not
     * pending, we block it in this thread (and we avoid changing signal action,
     * because it is per-process). */
    sigset_t pending;
    sigemptyset(&pending);
    sigpending(&pending);

    m_sigpipe_pending = sigismember(&pending, SIGPIPE);
    if (!m_sigpipe_pending) {
        sigset_t sigpipe_mask;
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);

        sigset_t blocked;
        sigemptyset(&blocked);
        pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &blocked);

        /* Maybe is was blocked already?  */
        m_sigpipe_unblock = !sigismember(&blocked, SIGPIPE);
    }
}

CSigpipe_scoped::~CSigpipe_scoped() {
    TRACE("Destruct CSigpipe_scoped");
    /* If SIGPIPE was pending already we do nothing. Otherwise, if it become
     * pending (i.e., we generated it), then we sigwait() it (thus clearing
     * pending status). Then we unblock SIGPIPE, but only if it were us who
     * blocked it. */
    if (!m_sigpipe_pending) {
        sigset_t sigpipe_mask;
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);

        // I cannot use sigtimedwait() because it isn't available on
        // macOS/OpenBSD. I workaround it with sigpending() and sigwait() to
        // ensure that sigwait() never blocks.
        sigset_t pending;
        while (true) {
            /* Protect ourselves from a situation when SIGPIPE was sent by
             * the user to the whole process, and was delivered to other
             * thread before we had a chance to wait for it. */
            sigemptyset(&pending);
            sigpending(&pending);
            if (sigismember(&pending, SIGPIPE)) {
                int sig; // Only return buffer, not used.
                sigwait(&sigpipe_mask, &sig);
            } else
                break;
        }

        if (m_sigpipe_unblock)
            pthread_sigmask(SIG_UNBLOCK, &sigpipe_mask, NULL);
    }
}

} // namespace upnplib
/// \endcond

#endif
