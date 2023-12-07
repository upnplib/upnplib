// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-07

#include <upnplib/connection.hpp>

namespace upnplib {

#if !defined __APPLE__ && !defined _MSC_VER
CSigpipe::CSigpipe() {
    ::sigemptyset(&m_sigpipe_mask);
    ::sigaddset(&m_sigpipe_mask, SIGPIPE);
}
#endif

void CSigpipe::suppress([[maybe_unused]] SOCKET sockfd) {
#ifdef _MSC_VER
    // Nothing to do for Microsoft Windows. It does not invoke a signal.
#elif __APPLE__
    // On MacOS I set the SO_NOSIGPIPE option for send() with the socket.
#else
    /*
      We want to ignore possible SIGPIPE that we can generate on write.
      SIGPIPE is delivered *synchronously* and *only* to the thread doing
      the write. So if it is reported as already pending (which means the
      thread blocks it), then we do nothing: if we generate SIGPIPE, it will
      be merged with the pending one (there's no queuing), and that suits us
      well. If it is not pending, we block it in this thread (and we avoid
      changing signal action, because it is per-process).
    */
    ::sigset_t pending;
    ::sigemptyset(&pending);
    ::sigpending(&pending);

    m_sigpipe_pending = ::sigismember(&pending, SIGPIPE);
    if (!m_sigpipe_pending) {
        ::sigset_t blocked;
        ::sigemptyset(&blocked);
        ::pthread_sigmask(SIG_BLOCK, &m_sigpipe_mask, &blocked);

        /* Maybe is was blocked already?  */
        m_sigpipe_unblock = !::sigismember(&blocked, SIGPIPE);
    }
#endif
}

void CSigpipe::restore() {
#if !defined __APPLE__ && !defined _MSC_VER
    /*
      If SIGPIPE was pending already we do nothing. Otherwise, if it become
      pending (i.e., we generated it), then we sigwait() it (thus clearing
      pending status). Then we unblock SIGPIPE, but only if it were us who
      blocked it.
    */
    if (!m_sigpipe_pending) {
        ::sigset_t pending;
        ::sigemptyset(&pending);
        ::sigpending(&pending);
        if (::sigismember(&pending, SIGPIPE)) {
            /*
              Protect ourselves from a situation when SIGPIPE was sent by
              the user to the whole process, and was delivered to other
              thread before we had a chance to wait for it.
            */
            static const ::timespec nowait = {0, 0};
            int sig{-1};
            do
                sig = ::sigtimedwait(&m_sigpipe_mask, NULL, &nowait);
            while (sig == -1 && errno == EINTR);
        }

        if (m_sigpipe_unblock)
            ::pthread_sigmask(SIG_UNBLOCK, &m_sigpipe_mask, NULL);
    }
#endif
}

} // namespace upnplib
