// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-02

#ifndef UPNP_THREADPOOL_HPP
#define UPNP_THREADPOOL_HPP

#include "ThreadPool.hpp"

namespace upnplib {

//###############################
// ThreadPool Interface         #
//###############################

class IThreadPool {
  public:
    virtual ~IThreadPool() {}
    virtual int ThreadPoolInit(ThreadPool* tp, ThreadPoolAttr* attr) = 0;
    virtual int ThreadPoolAddPersistent(ThreadPool* tp, ThreadPoolJob* job,
                                        int* jobId) = 0;
    virtual int ThreadPoolAdd(ThreadPool* tp, ThreadPoolJob* job,
                              int* jobId) = 0;
    virtual int ThreadPoolRemove(ThreadPool* tp, int jobId,
                                 ThreadPoolJob* out) = 0;
    virtual int ThreadPoolGetAttr(ThreadPool* tp, ThreadPoolAttr* out) = 0;
    virtual int ThreadPoolSetAttr(ThreadPool* tp, ThreadPoolAttr* attr) = 0;
    virtual int ThreadPoolShutdown(ThreadPool* tp) = 0;
    virtual int TPAttrInit(ThreadPoolAttr* attr) = 0;
    virtual int TPJobInit(ThreadPoolJob* job, start_routine func,
                          void* arg) = 0;
    virtual int TPJobSetPriority(ThreadPoolJob* job,
                                 ThreadPriority priority) = 0;
    virtual int TPJobSetFreeFunction(ThreadPoolJob* job, free_routine func) = 0;
    virtual int TPAttrSetMaxThreads(ThreadPoolAttr* attr, int maxThreads) = 0;
    virtual int TPAttrSetMinThreads(ThreadPoolAttr* attr, int minThreads) = 0;
    virtual int TPAttrSetStackSize(ThreadPoolAttr* attr, size_t stackSize) = 0;
    virtual int TPAttrSetIdleTime(ThreadPoolAttr* attr, int idleTime) = 0;
    virtual int TPAttrSetJobsPerThread(ThreadPoolAttr* attr,
                                       int jobsPerThread) = 0;
    virtual int TPAttrSetStarvationTime(ThreadPoolAttr* attr,
                                        int starvationTime) = 0;
    virtual int TPAttrSetSchedPolicy(ThreadPoolAttr* attr,
                                     PolicyType schedPolicy) = 0;
    virtual int TPAttrSetMaxJobsTotal(ThreadPoolAttr* attr,
                                      int maxJobsTotal) = 0;
    virtual void ThreadPoolPrintStats(ThreadPoolStats* stats) = 0;
    virtual int ThreadPoolGetStats(ThreadPool* tp, ThreadPoolStats* stats) = 0;
    virtual int gettimeofday(struct timeval* tv, struct timezone* tz) = 0;
};

class CThreadPool : public IThreadPool {
  public:
    virtual ~CThreadPool() {}

    int ThreadPoolInit(ThreadPool* tp, ThreadPoolAttr* attr) override {
        return ::ThreadPoolInit(tp, attr);
    }
    int ThreadPoolAddPersistent(ThreadPool* tp, ThreadPoolJob* job,
                                int* jobId) override {
        return ::ThreadPoolAddPersistent(tp, job, jobId);
    }
    int ThreadPoolAdd(ThreadPool* tp, ThreadPoolJob* job, int* jobId) override {
        return ::ThreadPoolAdd(tp, job, jobId);
    }
    int ThreadPoolRemove(ThreadPool* tp, int jobId,
                         ThreadPoolJob* out) override {
        return ::ThreadPoolRemove(tp, jobId, out);
    }
    int ThreadPoolGetAttr(ThreadPool* tp, ThreadPoolAttr* out) override {
        return ::ThreadPoolGetAttr(tp, out);
    }
    int ThreadPoolSetAttr(ThreadPool* tp, ThreadPoolAttr* attr) override {
        return ::ThreadPoolSetAttr(tp, attr);
    }
    int ThreadPoolShutdown(ThreadPool* tp) override {
        return ::ThreadPoolShutdown(tp);
    }
    int TPAttrInit(ThreadPoolAttr* attr) override { return ::TPAttrInit(attr); }
    int TPJobInit(ThreadPoolJob* job, start_routine func, void* arg) override {
        return ::TPJobInit(job, func, arg);
    }
    int TPJobSetPriority(ThreadPoolJob* job, ThreadPriority priority) override {
        return ::TPJobSetPriority(job, priority);
    }
    int TPJobSetFreeFunction(ThreadPoolJob* job, free_routine func) override {
        return ::TPJobSetFreeFunction(job, func);
    }
    int TPAttrSetMaxThreads(ThreadPoolAttr* attr, int maxThreads) override {
        return ::TPAttrSetMaxThreads(attr, maxThreads);
    }
    int TPAttrSetMinThreads(ThreadPoolAttr* attr, int minThreads) override {
        return ::TPAttrSetMinThreads(attr, minThreads);
    }
    int TPAttrSetStackSize(ThreadPoolAttr* attr, size_t stackSize) override {
        return ::TPAttrSetStackSize(attr, stackSize);
    }
    int TPAttrSetIdleTime(ThreadPoolAttr* attr, int idleTime) override {
        return ::TPAttrSetIdleTime(attr, idleTime);
    }
    int TPAttrSetJobsPerThread(ThreadPoolAttr* attr,
                               int jobsPerThread) override {
        return ::TPAttrSetJobsPerThread(attr, jobsPerThread);
    }
    int TPAttrSetStarvationTime(ThreadPoolAttr* attr,
                                int starvationTime) override {
        return ::TPAttrSetStarvationTime(attr, starvationTime);
    }
    int TPAttrSetSchedPolicy(ThreadPoolAttr* attr,
                             PolicyType schedPolicy) override {
        return ::TPAttrSetSchedPolicy(attr, schedPolicy);
    }
    int TPAttrSetMaxJobsTotal(ThreadPoolAttr* attr, int maxJobsTotal) override {
        return ::TPAttrSetMaxJobsTotal(attr, maxJobsTotal);
    }
    void ThreadPoolPrintStats(ThreadPoolStats* stats) override {
        return ::ThreadPoolPrintStats(stats);
    }
    int ThreadPoolGetStats(ThreadPool* tp, ThreadPoolStats* stats) override {
        return ::ThreadPoolGetStats(tp, stats);
    }
    int gettimeofday(struct timeval* tv, struct timezone* tz) override {
        return ::gettimeofday(tv, tz);
    }
};

} // namespace upnplib

#endif // UPNP_THREADPOOL_HPP
