#ifndef INTERFACE_THREADPOOL_HPP
#define INTERFACE_THREADPOOL_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-06

#include <ThreadPool.hpp>

//###############################
// ThreadPool Interface         #
//###############################

// clang-format off
class IThreadPool {
  public:
    virtual int ThreadPoolInit(ThreadPool* tp, ThreadPoolAttr* attr) = 0;
    virtual int ThreadPoolAddPersistent(ThreadPool* tp, ThreadPoolJob* job, int* jobId) = 0;
    virtual int ThreadPoolAdd(ThreadPool* tp, ThreadPoolJob* job, int* jobId) = 0;
    virtual int ThreadPoolRemove(ThreadPool* tp, int jobId, ThreadPoolJob* out) = 0;
    virtual int ThreadPoolGetAttr(ThreadPool* tp, ThreadPoolAttr* out) = 0;
    virtual int ThreadPoolSetAttr(ThreadPool* tp, ThreadPoolAttr* attr) = 0;
    virtual int ThreadPoolShutdown(ThreadPool* tp) = 0;
    virtual int TPAttrInit(ThreadPoolAttr* attr) = 0;
    virtual int TPJobInit(ThreadPoolJob* job, start_routine func, void* arg) = 0;
    virtual int TPJobSetPriority(ThreadPoolJob* job, ThreadPriority priority) = 0;
    virtual int TPJobSetFreeFunction(ThreadPoolJob* job, free_routine func) = 0;
    virtual int TPAttrSetMaxThreads(ThreadPoolAttr* attr, int maxThreads) = 0;
    virtual int TPAttrSetMinThreads(ThreadPoolAttr* attr, int minThreads) = 0;
    virtual int TPAttrSetStackSize(ThreadPoolAttr* attr, size_t stackSize) = 0;
    virtual int TPAttrSetIdleTime(ThreadPoolAttr* attr, int idleTime) = 0;
    virtual int TPAttrSetJobsPerThread(ThreadPoolAttr* attr, int jobsPerThread) = 0;
    virtual int TPAttrSetStarvationTime(ThreadPoolAttr* attr, int starvationTime) = 0;
    virtual int TPAttrSetSchedPolicy(ThreadPoolAttr* attr, PolicyType schedPolicy) = 0;
    virtual int TPAttrSetMaxJobsTotal(ThreadPoolAttr* attr, int maxJobsTotal) = 0;
    virtual void ThreadPoolPrintStats(ThreadPoolStats* stats) = 0;
    virtual int ThreadPoolGetStats(ThreadPool* tp, ThreadPoolStats* stats) = 0;
    virtual int gettimeofday(struct timeval* tv, struct timezone* tz) = 0;
};
// clang-format on

#endif // INTERFACE_THREADPOOL_HPP
