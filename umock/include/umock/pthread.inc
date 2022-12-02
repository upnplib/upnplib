// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

#include "upnplib/visibility.hpp"
#include "pthread.h" // To find pthreads4w don't use <pthread.h>

namespace umock {

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
    PthreadReal pthread_realObj; // already done below
    Pthread(&pthread_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class PthreadMock : public PthreadInterface { ...; MOCK_METHOD(...) };
        PthreadMock pthread_mockObj;
        Pthread pthread_injectObj(&pthread_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pthread_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Pthread {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Pthread(PthreadReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Pthread(PthreadInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Pthread();

    // Methods
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
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Pthread::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline PthreadInterface* m_ptr_workerObj;
    PthreadInterface* m_ptr_oldObj{};
};

} // namespace umock
// vim: syntax=cpp
