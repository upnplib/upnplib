// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-02

#ifndef UPNP_PTHREADIF_HPP
#define UPNP_PTHREADIF_HPP

#include <pthread.h>

namespace upnplib {

class Bpthread {
    // Base class to pthread system calls
  public:
    virtual ~Bpthread() {}

    virtual int pthread_mutex_init(pthread_mutex_t* mutex,
                                   const pthread_mutexattr_t* mutexattr) {
        return ::pthread_mutex_init(mutex, mutexattr);
    }
    virtual int pthread_mutex_lock(pthread_mutex_t* mutex) {
        return ::pthread_mutex_lock(mutex);
    }
    virtual int pthread_mutex_unlock(pthread_mutex_t* mutex) {
        return ::pthread_mutex_unlock(mutex);
    }
    virtual int pthread_mutex_destroy(pthread_mutex_t* mutex) {
        return ::pthread_mutex_destroy(mutex);
    }

    virtual int pthread_cond_init(pthread_cond_t* cond,
                                  pthread_condattr_t* cond_attr) {
        return ::pthread_cond_init(cond, cond_attr);
    }
    virtual int pthread_cond_signal(pthread_cond_t* cond) {
        return ::pthread_cond_signal(cond);
    }
    virtual int pthread_cond_broadcast(pthread_cond_t* cond) {
        return ::pthread_cond_broadcast(cond);
    }
    virtual int pthread_cond_wait(pthread_cond_t* cond,
                                  pthread_mutex_t* mutex) {
        return ::pthread_cond_wait(cond, mutex);
    }
    virtual int pthread_cond_timedwait(pthread_cond_t* cond,
                                       pthread_mutex_t* mutex,
                                       const struct timespec* abstime) {
        return ::pthread_cond_timedwait(cond, mutex, abstime);
    }
    virtual int pthread_cond_destroy(pthread_cond_t* cond) {
        return ::pthread_cond_destroy(cond);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bpthread* pthread_h;

// In the production code you just prefix the old system call with
// 'upnplib::pthread_h->' so the new call looks like this:
//  upnplib::pthread_h->pthread_mutex_init(...)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_pthread : public Bpthread {
    // Class to mock the free system functions.
    Bpthread* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_pthread() {
        m_oldptr = pthread_h;
        pthread_h = this;
    }
    virtual ~Mock_pthread() { pthread_h = m_oldptr; }

    MOCK_METHOD(int, pthread_mutex_init,
                (pthread_mutex_t * mutex,
                 const pthread_mutexattr_t* mutexattr), (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex), (override));

    MOCK_METHOD(int, pthread_cond_init,
                (pthread_cond_t * cond, pthread_condattr_t* cond_attr), (override));
    MOCK_METHOD(int, pthread_cond_signal, (pthread_cond_t * cond), (override));
    MOCK_METHOD(int, pthread_cond_broadcast, (pthread_cond_t * cond), (override));
    MOCK_METHOD(int, pthread_cond_wait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_cond_timedwait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex,
                 const struct timespec* abstime), (override));
    MOCK_METHOD(int, pthread_cond_destroy, (pthread_cond_t * cond), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_pthread m_mocked_pthread;

 * and call it with: m_mocked_pthread.pthread_mutex_init(...)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNP_PTHREADIF_HPP
