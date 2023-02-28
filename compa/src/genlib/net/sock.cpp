// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-04

#include <compa/sock.hpp>
#include <upnp.hpp>
#include <cerrno>
#ifndef _WIN32
#include <csignal>
#endif

#ifdef UPNP_ENABLE_OPEN_SSL
UPNPLIB_EXTERN SSL_CTX* gSslCtx;
#endif

namespace compa {

#if !defined __APPLE__ && !defined _WIN32
class Csigpipe {
    // Taking the idea and example for this SIGPIPE handler from
    // https://stackoverflow.com/a/2347848/5014688 and adapt it. Thanks to
    // kroki. --Ingo

  private:
    sigset_t sigpipe_mask;
    bool sigpipe_pending;
    bool sigpipe_unblock;

  public:
    Csigpipe() {
        // The only error that can be returned in errno is 'EINVAL signum is not
        // a valid signal.' SIGPIPE is always a valid signum, so no error
        // handling is required.
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);
    }

    void suppress(void) {
        /*
          We want to ignore possible SIGPIPE that we can generate on write.
          SIGPIPE is delivered *synchronously* and *only* to the thread doing
          the write. So if it is reported as already pending (which means the
          thread blocks it), then we do nothing: if we generate SIGPIPE, it will
          be merged with the pending one (there's no queuing), and that suits us
          well. If it is not pending, we block it in this thread (and we avoid
          changing signal action, because it is per-process).
        */
        sigset_t pending;
        sigemptyset(&pending);
        sigpending(&pending);

        sigpipe_pending = sigismember(&pending, SIGPIPE);
        if (!sigpipe_pending) {
            sigset_t blocked;
            sigemptyset(&blocked);
            pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &blocked);

            /* Maybe is was blocked already?  */
            sigpipe_unblock = !sigismember(&blocked, SIGPIPE);
        }
    }

    void restore(void) {
        /*
          If SIGPIPE was pending already we do nothing. Otherwise, if it become
          pending (i.e., we generated it), then we sigwait() it (thus clearing
          pending status). Then we unblock SIGPIPE, but only if it were us who
          blocked it.
        */
        if (!sigpipe_pending) {
            sigset_t pending;
            sigemptyset(&pending);
            sigpending(&pending);
            if (sigismember(&pending, SIGPIPE)) {
                /*
                  Protect ourselves from a situation when SIGPIPE was sent by
                  the user to the whole process, and was delivered to other
                  thread before we had a chance to wait for it.
                */
                static const struct timespec nowait = {0, 0};
                int sig{-1};
                do
                    sig = sigtimedwait(&sigpipe_mask, NULL, &nowait);
                while (sig == -1 && errno == EINTR);
            }

            if (sigpipe_unblock)
                pthread_sigmask(SIG_UNBLOCK, &sigpipe_mask, NULL);
        }
    }
};
#endif


#ifdef UPNP_ENABLE_OPEN_SSL
int Csock::sock_ssl_connect(SOCKINFO* info) {
    int status{};
    info->ssl = SSL_new(gSslCtx);
    if (!info->ssl) {
        return UPNP_E_SOCKET_ERROR;
    }
    // Due to man page there is no problem with type cast (int)
    status = SSL_set_fd(info->ssl, (int)info->socket);
    if (status == 1) {
#if defined _WIN32 || defined __APPLE__
        status = SSL_connect(info->ssl);
#else
        Csigpipe sigpipe;
        sigpipe.suppress();
        status = SSL_connect(info->ssl);
        sigpipe.restore();
#endif
    }
    if (status == 1) {
        return UPNP_E_SUCCESS;
    }
    return UPNP_E_SOCKET_ERROR;
}
#endif

} // namespace compa
