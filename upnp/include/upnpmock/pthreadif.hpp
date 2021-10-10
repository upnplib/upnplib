// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-10

#ifndef UPNP_PTHREAD_H
#define UPNP_PTHREAD_H

#include <pthread.h>

namespace upnp {

class Ipthread {
    // Interface to pthread system calls
  public:
    virtual ~Ipthread() {}
    virtual int pthread_mutex_init(pthread_mutex_t* mutex,
                                   const pthread_mutexattr_t* mutexattr) = 0;
    virtual int pthread_mutex_lock(pthread_mutex_t* mutex) = 0;
    virtual int pthread_mutex_unlock(pthread_mutex_t* mutex) = 0;
    virtual int pthread_mutex_destroy(pthread_mutex_t* mutex) = 0;
};

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
Ipthread* pthreadif;

class Cpthread : public Ipthread {
    // Real class to call the system functions.
  public:
    virtual ~Cpthread() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cpthread() { pthreadif = this; }

    int pthread_mutex_init(pthread_mutex_t* mutex,
                           const pthread_mutexattr_t* mutexattr) override {
        return ::pthread_mutex_init(mutex, mutexattr);
    }

    int pthread_mutex_lock(pthread_mutex_t* mutex) override {
        return ::pthread_mutex_lock(mutex);
    }

    int pthread_mutex_unlock(pthread_mutex_t* mutex) override {
        return ::pthread_mutex_unlock(mutex);
    }

    int pthread_mutex_destroy(pthread_mutex_t* mutex) override {
        return ::pthread_mutex_destroy(mutex);
    }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer pthreadif (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cpthread pthreadObj;

// In the production code you must call it with, e.g.:
// pthreadif->pthread_mutex_init(...)

/*
 * The following class should be coppied to the test source. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_pthread : public Ipthread {
// Class to mock the free system functions.
  public:
    virtual ~Mock_pthread() {}
    Mock_pthread() { pthreadif = this; }
    MOCK_METHOD(int, pthread_mutex_init, (pthread_mutex_t* mutex,
                const pthread_mutexattr_t* mutexattr), (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t* mutex), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_pthread mocked_pthread;

 *  and call it with: mocked_pthread.pthread_mutex_init(...) (prefered)
 *  or                    pthreadif->pthread_mutex_init(...)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_PTHREAD_H
