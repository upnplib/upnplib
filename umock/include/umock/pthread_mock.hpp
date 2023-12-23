#ifndef UMOCK_PTHREAD_MOCK_HPP
#define UMOCK_PTHREAD_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-25

#include <umock/pthread.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API PthreadMock : public umock::PthreadInterface {
  public:
    PthreadMock();
    virtual ~PthreadMock() override;
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(int, pthread_mutex_init,
                (pthread_mutex_t * mutex, const pthread_mutexattr_t* mutexattr),
                (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_cond_init,
                (pthread_cond_t * cond, pthread_condattr_t* cond_attr),
                (override));
    MOCK_METHOD(int, pthread_cond_signal, (pthread_cond_t * cond), (override));
    MOCK_METHOD(int, pthread_cond_broadcast, (pthread_cond_t * cond),
                (override));
    MOCK_METHOD(int, pthread_cond_wait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex), (override));
    MOCK_METHOD(int, pthread_cond_timedwait,
                (pthread_cond_t * cond, pthread_mutex_t* mutex,
                 const struct timespec* abstime),
                (override));
    MOCK_METHOD(int, pthread_cond_destroy, (pthread_cond_t * cond), (override));
    ENABLE_MSVC_WARN
};

} // namespace umock

#endif // UMOCK_PTHREAD_MOCK_HPP
