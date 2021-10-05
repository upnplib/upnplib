#ifndef UPNP_PTHREAD_H
#define UPNP_PTHREAD_H

#include <pthread.h>

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

class Cpthread : public Ipthread {
    // Real class to call the system functions.
  public:
    virtual ~Cpthread() {}

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
// This is the instance to call the system functions.
// It is initialized by default.
Cpthread pthreadObj;
Ipthread* pthread = &pthreadObj;

// In the production code you must call it with, e.g.:
// pthread->pthread_mutex_init(...)

/*
 * The following class should be coppied to the test source.

class Mock_pthread : public Ipthread {
// Class to mock the free system functions.
  public:
    virtual ~Mock_pthread() {}
    MOCK_METHOD(int, pthread_mutex_init, (pthread_mutex_t* mutex,
                const pthread_mutexattr_t* mutexattr), (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t* mutex), (override));
};

 * In a test macro you will instantiate the Mock class and overwrite the pointer
 * "Ipthread* pthread" (look above) to the interface so it points to the mocking
 * class now, e.g.:
Mock_pthread mocked_pthread;
pthread = &mocked_pthread;
 *  and call it with: mocked_pthread.pthread_mutex_init(...) (prefered)
 *  or                      pthread->pthread_mutex_init(...)
 * clang-format on
*/

#endif // UPNP_PTHREAD_H
