#ifndef MOCKING_PTHREAD_HPP
#define MOCKING_PTHREAD_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-20

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#include "pthread.h"      // To find pthreads4w don't use <pthread.h>

namespace upnplib {
namespace mocking {

// clang-format off
class PthreadInterface {
  public:
    virtual ~PthreadInterface() {}

    virtual int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr) = 0;
    virtual int pthread_mutex_lock(pthread_mutex_t* mutex) = 0;
    virtual int pthread_mutex_unlock(pthread_mutex_t* mutex) = 0;
    virtual int pthread_mutex_destroy(pthread_mutex_t* mutex) = 0;

    virtual int pthread_cond_init(pthread_cond_t* cond, pthread_condattr_t* cond_attr) = 0;
    virtual int pthread_cond_signal(pthread_cond_t* cond) = 0;
    virtual int pthread_cond_broadcast(pthread_cond_t* cond) = 0;
    virtual int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) = 0;
    virtual int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime) = 0;
    virtual int pthread_cond_destroy(pthread_cond_t* cond) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class PthreadReal : public PthreadInterface {
  public:
    virtual ~PthreadReal() override {}

    int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr) override;
    int pthread_mutex_lock(pthread_mutex_t* mutex) override;
    int pthread_mutex_unlock(pthread_mutex_t* mutex) override;
    int pthread_mutex_destroy(pthread_mutex_t* mutex) override;

    int pthread_cond_init(pthread_cond_t* cond, pthread_condattr_t* cond_attr) override;
    int pthread_cond_signal(pthread_cond_t* cond) override;
    int pthread_cond_broadcast(pthread_cond_t* cond) override;
    int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) override;
    int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime) override;
    int pthread_cond_destroy(pthread_cond_t* cond) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    PthreadReal pthread_realObj;
    Pthread(&pthread_realObj;
    { // Other scope, e.g. within a gtest
        class PthreadMock { ...; MOCK_METHOD(...) };
        PthreadMock pthread_mockObj;
        Pthread(&pthread_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class EXPORT_SPEC Pthread {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Pthread(PthreadReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Pthread(PthreadInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Pthread();

    virtual int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr);
    virtual int pthread_mutex_lock(pthread_mutex_t* mutex);
    virtual int pthread_mutex_unlock(pthread_mutex_t* mutex);
    virtual int pthread_mutex_destroy(pthread_mutex_t* mutex);

    virtual int pthread_cond_init(pthread_cond_t* cond, pthread_condattr_t* cond_attr);
    virtual int pthread_cond_signal(pthread_cond_t* cond);
    virtual int pthread_cond_broadcast(pthread_cond_t* cond);
    virtual int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);
    virtual int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime);
    virtual int pthread_cond_destroy(pthread_cond_t* cond);
    // clang-format on

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Pthread::m_ptr_workerObj.
    EXPORT_SPEC_LOCAL static inline PthreadInterface* m_ptr_workerObj;
    PthreadInterface* m_ptr_oldObj{};
};

extern Pthread EXPORT_SPEC pthread_h;

} // namespace mocking
} // namespace upnplib

#endif // MOCKING_PTHREAD_HPP
