// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-16

#include "upnplib/mocking/pthread.inc"

namespace upnplib::mocking {

int PthreadReal::pthread_mutex_init(pthread_mutex_t* mutex,
                                    const pthread_mutexattr_t* mutexattr) {
    return ::pthread_mutex_init(mutex, mutexattr);
}
int PthreadReal::pthread_mutex_lock(pthread_mutex_t* mutex) {
    return ::pthread_mutex_lock(mutex);
}
int PthreadReal::pthread_mutex_unlock(pthread_mutex_t* mutex) {
    return ::pthread_mutex_unlock(mutex);
}
int PthreadReal::pthread_mutex_destroy(pthread_mutex_t* mutex) {
    return ::pthread_mutex_destroy(mutex);
}

int PthreadReal::pthread_cond_init(pthread_cond_t* cond,
                                   pthread_condattr_t* cond_attr) {
    return ::pthread_cond_init(cond, cond_attr);
}
int PthreadReal::pthread_cond_signal(pthread_cond_t* cond) {
    return ::pthread_cond_signal(cond);
}
int PthreadReal::pthread_cond_broadcast(pthread_cond_t* cond) {
    return ::pthread_cond_broadcast(cond);
}
int PthreadReal::pthread_cond_wait(pthread_cond_t* cond,
                                   pthread_mutex_t* mutex) {
    return ::pthread_cond_wait(cond, mutex);
}
int PthreadReal::pthread_cond_timedwait(pthread_cond_t* cond,
                                        pthread_mutex_t* mutex,
                                        const struct timespec* abstime) {
    return ::pthread_cond_timedwait(cond, mutex, abstime);
}
int PthreadReal::pthread_cond_destroy(pthread_cond_t* cond) {
    return ::pthread_cond_destroy(cond);
}

// This constructor is used to inject the pointer to the real function.
Pthread::Pthread(PthreadReal* a_ptr_realObj) {
    m_ptr_workerObj = (PthreadInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Pthread::Pthread(PthreadInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Pthread::~Pthread() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Pthread::pthread_mutex_init(pthread_mutex_t* mutex,
                                const pthread_mutexattr_t* mutexattr) {
    return m_ptr_workerObj->pthread_mutex_init(mutex, mutexattr);
}
int Pthread::pthread_mutex_lock(pthread_mutex_t* mutex) {
    return m_ptr_workerObj->pthread_mutex_lock(mutex);
}
int Pthread::pthread_mutex_unlock(pthread_mutex_t* mutex) {
    return m_ptr_workerObj->pthread_mutex_unlock(mutex);
}
int Pthread::pthread_mutex_destroy(pthread_mutex_t* mutex) {
    return m_ptr_workerObj->pthread_mutex_destroy(mutex);
}

int Pthread::pthread_cond_init(pthread_cond_t* cond,
                               pthread_condattr_t* cond_attr) {
    return m_ptr_workerObj->pthread_cond_init(cond, cond_attr);
}
int Pthread::pthread_cond_signal(pthread_cond_t* cond) {
    return m_ptr_workerObj->pthread_cond_signal(cond);
}
int Pthread::pthread_cond_broadcast(pthread_cond_t* cond) {
    return m_ptr_workerObj->pthread_cond_broadcast(cond);
}
int Pthread::pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    return m_ptr_workerObj->pthread_cond_wait(cond, mutex);
}
int Pthread::pthread_cond_timedwait(pthread_cond_t* cond,
                                    pthread_mutex_t* mutex,
                                    const struct timespec* abstime) {
    return m_ptr_workerObj->pthread_cond_timedwait(cond, mutex, abstime);
}
int Pthread::pthread_cond_destroy(pthread_cond_t* cond) {
    return m_ptr_workerObj->pthread_cond_destroy(cond);
}

// On program start create an object and inject pointer to the real function.
// This will exist until program end.
PthreadReal pthread_realObj;
UPNPLIB_API Pthread pthread_h(&pthread_realObj);

} // namespace upnplib::mocking
