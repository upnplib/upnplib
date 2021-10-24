// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-31

#include "port.hpp"
#include "gmock/gmock.h"

#include "FreeList.cpp"
#include "LinkedList.cpp"

#include "ThreadPool.cpp"

namespace upnp {

//###############################
// ThreadPool Testsuite         #
//###############################

// Interface for the ThreadPool module
// -----------------------------------
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

//
// Testsuite for the ThreadPool module
// -----------------------------------
class ThreadPoolTestSuite : public ::testing::Test {
  protected:
    CThreadPool m_tpObj{}; // ThreadPool Object

    ThreadPool m_tp{};         // Structure for a threadpool
    ThreadPoolJob m_TPJob{};   // Structure for a threadpool job
    ThreadPoolAttr m_TPAttr{}; // Structure for a threadpool attribute
};

start_routine start_function() { return 0; }

TEST_F(ThreadPoolTestSuite, normal_add_and_remove_job_on_threadpool) {
    int jobId{};
    ThreadPoolJob removedJob{};

    // Initialize threadpool and job
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    EXPECT_EQ(
        m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function, nullptr), 0);
    // Add and remove job
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, &m_TPJob, &jobId), 0);
    // std::cout << "DEBUG: EOUTOFMEM is: " << EOUTOFMEM << "\n";
    EXPECT_EQ(m_tpObj.ThreadPoolRemove(&m_tp, jobId, &removedJob), 0);
    EXPECT_EQ(removedJob.jobId, jobId);
}

TEST_F(ThreadPoolTestSuite, init_job_and_set_job_priority) {
    EXPECT_EQ(
        m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function, nullptr), 0);

    EXPECT_EQ(m_tpObj.TPJobSetPriority(&m_TPJob, LOW_PRIORITY), 0);
    EXPECT_EQ(m_tpObj.TPJobSetPriority(&m_TPJob, MED_PRIORITY), 0);
    EXPECT_EQ(m_tpObj.TPJobSetPriority(&m_TPJob, HIGH_PRIORITY), 0);

    EXPECT_EQ(m_tpObj.TPJobSetPriority(nullptr, (ThreadPriority)-1), EINVAL);
    EXPECT_EQ(m_tpObj.TPJobSetPriority(nullptr, LOW_PRIORITY), EINVAL);
    EXPECT_EQ(m_tpObj.TPJobSetPriority(&m_TPJob, (ThreadPriority)-1), EINVAL);
    EXPECT_EQ(m_tpObj.TPJobSetPriority(&m_TPJob, (ThreadPriority)3), EINVAL);
}

TEST_F(ThreadPoolTestSuite, set_job_free_function) {
    EXPECT_EQ(
        m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function, nullptr), 0);

    EXPECT_EQ(m_tpObj.TPJobSetFreeFunction(nullptr, DEFAULT_FREE_ROUTINE),
              EINVAL);
    EXPECT_EQ(m_tpObj.TPJobSetFreeFunction(&m_TPJob, DEFAULT_FREE_ROUTINE), 0);
}

TEST_F(ThreadPoolTestSuite, init_threadpool_attribute) {
    EXPECT_EQ(m_tpObj.TPAttrInit(nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.TPAttrInit(&m_TPAttr), 0);
}

TEST_F(ThreadPoolTestSuite, set_maximal_threads_to_attribute) {
    EXPECT_EQ(TPAttrSetMaxThreads(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, 0), 0);
    EXPECT_EQ(m_TPAttr.minThreads, 0);
    EXPECT_EQ(m_TPAttr.maxThreads, 0);

    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, 1), 0);
    EXPECT_EQ(m_TPAttr.minThreads, 0);
    EXPECT_EQ(m_TPAttr.maxThreads, 1);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set maxThreads < 0"
              << " or < minThreads.\n";
    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, -1), 0);
    m_TPAttr.minThreads = 2;
    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, 1), 0);
#else
    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set maxThreads < 0.";
    m_TPAttr.minThreads = 2;
    EXPECT_EQ(TPAttrSetMaxThreads(&m_TPAttr, 1), EINVAL)
        << "# It should not be possible to set maxThreads < minThreads.";
#endif
    EXPECT_EQ(m_TPAttr.minThreads, 2);
    EXPECT_EQ(m_TPAttr.maxThreads, 1);
}

TEST_F(ThreadPoolTestSuite, set_minimal_threads_to_attribute) {
    EXPECT_EQ(TPAttrSetMinThreads(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, 0), 0);
    EXPECT_EQ(m_TPAttr.minThreads, 0);
    EXPECT_EQ(m_TPAttr.maxThreads, 0);

    m_TPAttr.maxThreads = 2;
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, 1), 0);
    EXPECT_EQ(m_TPAttr.minThreads, 1);
    EXPECT_EQ(m_TPAttr.maxThreads, 2);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set minThreads < 0"
              << " or > maxThreads.\n";
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, -1), 0);
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, 3), 0);
    EXPECT_EQ(m_TPAttr.minThreads, 3);
#else
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set minThreads < 0.";
    EXPECT_EQ(TPAttrSetMinThreads(&m_TPAttr, 3), EINVAL)
        << "# It should not be possible to set minThreads > maxThreads.";
    EXPECT_EQ(m_TPAttr.minThreads, 1)
        << "# Wrong settings should not modify old minThreads value.";
#endif
    EXPECT_EQ(m_TPAttr.maxThreads, 2);
}

TEST_F(ThreadPoolTestSuite, set_stack_size_to_attribute) {
    EXPECT_EQ(TPAttrSetStackSize(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetStackSize(&m_TPAttr, 0), 0);
    EXPECT_EQ(TPAttrSetStackSize(&m_TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set StackSize < 0.\n";
    EXPECT_EQ(TPAttrSetStackSize(&m_TPAttr, -1), 0);
    EXPECT_EQ(m_TPAttr.stackSize, -1);
#else
    EXPECT_EQ(TPAttrSetStackSize(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set StackSize < 0.";
    EXPECT_EQ(m_TPAttr.stackSize, 1)
        << "# Wrong settings should not modify old stackSize value.";
#endif
}

TEST_F(ThreadPoolTestSuite, set_idle_time_to_attribute) {
    EXPECT_EQ(TPAttrSetIdleTime(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetIdleTime(&m_TPAttr, 0), 0);
    EXPECT_EQ(TPAttrSetIdleTime(&m_TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set IdleTime < 0.\n";
    EXPECT_EQ(TPAttrSetIdleTime(&m_TPAttr, -1), 0);
    EXPECT_EQ(m_TPAttr.maxIdleTime, -1);
#else
    EXPECT_EQ(TPAttrSetIdleTime(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set IdleTime < 0.";
    EXPECT_EQ(m_TPAttr.maxIdleTime, 1)
        << "# Wrong settings should not modify old maxIdleTime value.";
#endif
}

TEST_F(ThreadPoolTestSuite, set_jobs_per_thread_to_attribute) {
    EXPECT_EQ(TPAttrSetJobsPerThread(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetJobsPerThread(&m_TPAttr, 0), 0);
    EXPECT_EQ(TPAttrSetJobsPerThread(&m_TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set JobsPerThread < 0.\n";
    EXPECT_EQ(TPAttrSetJobsPerThread(&m_TPAttr, -1), 0);
    EXPECT_EQ(m_TPAttr.jobsPerThread, -1);
#else
    EXPECT_EQ(TPAttrSetJobsPerThread(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set JobsPerThread < 0.";
    EXPECT_EQ(m_TPAttr.jobsPerThread, 1)
        << "# Wrong settings should not modify old JobsPerThread value.";
#endif
}

TEST_F(ThreadPoolTestSuite, set_starvation_time_to_attribute) {
    EXPECT_EQ(TPAttrSetStarvationTime(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetStarvationTime(&m_TPAttr, 0), 0);
    EXPECT_EQ(TPAttrSetStarvationTime(&m_TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout
        << "  BUG! It should not be possible to set StarvationTime < 0.\n";
    EXPECT_EQ(TPAttrSetStarvationTime(&m_TPAttr, -1), 0);
    EXPECT_EQ(m_TPAttr.starvationTime, -1);
#else
    EXPECT_EQ(TPAttrSetStarvationTime(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set StarvationTime < 0.";
    EXPECT_EQ(m_TPAttr.starvationTime, 1)
        << "# Wrong settings should not modify old starvationTime value.";
#endif
}

TEST_F(ThreadPoolTestSuite, set_scheduling_policy_to_attribute) {
    // std::cout << "SCHED_OTHER: " << SCHED_OTHER
    //     << ", SCHED_IDLE: " << SCHED_IDLE
    //     << ", SCHED_BATCH: " << SCHED_BATCH << ", SCHED_FIFO: " << SCHED_FIFO
    //     << ", SCHED_RR: " << SCHED_RR
    //     << ", SCHED_DEADLINE; " << SCHED_DEADLINE << "\n";

    EXPECT_EQ(TPAttrSetSchedPolicy(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetSchedPolicy(&m_TPAttr, SCHED_OTHER), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! Only SCHED_OTHER, SCHED_IDLE, SCHED_BATCH, SCHED_FIFO,"
              << " SCHED_RR or SCHED_DEADLINE should be valid.\n";
    EXPECT_EQ(TPAttrSetSchedPolicy(&m_TPAttr, 0x5a5a), 0);
    EXPECT_EQ(m_TPAttr.schedPolicy, 0x5a5a);
#else
    EXPECT_EQ(TPAttrSetSchedPolicy(&m_TPAttr, 0x5a5a), EINVAL)
        << "# Only SCHED_OTHER, SCHED_IDLE, SCHED_BATCH, SCHED_FIFO,"
        << " SCHED_RR or SCHED_DEADLINE should be valid.";
    EXPECT_EQ(m_TPAttr.schedPolicy, SCHED_OTHER)
        << "# Wrong settings should not modify old schedPolicy value.";
#endif
}

TEST_F(ThreadPoolTestSuite, set_max_jobs_qeued_totally_to_attribute) {
    EXPECT_EQ(TPAttrSetMaxJobsTotal(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&m_TPAttr, 0), 0);
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&m_TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set MaxJobsTotal < 0.\n";
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&m_TPAttr, -1), 0);
    EXPECT_EQ(m_TPAttr.maxJobsTotal, -1);
#else
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&m_TPAttr, -1), EINVAL)
        << "# It should not be possible to set MaxJobsTotal < 0.";
    EXPECT_EQ(m_TPAttr.maxJobsTotal, 1)
        << "# Wrong settings should not modify old maxJobsTotal value.";
#endif
}

TEST_F(ThreadPoolTestSuite, normal_threadpool_init_and_shutdown) {
    EXPECT_EQ(m_tpObj.ThreadPoolInit(nullptr, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    // process unit
    EXPECT_EQ(m_tpObj.ThreadPoolShutdown(nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolShutdown(&m_tp), 0);
}

TEST_F(ThreadPoolTestSuite,
       add_persistent_job_to_threadpool_with_uninitialized_job) {
#ifdef OLD_TEST
    GTEST_SKIP() << "  BUG! Unit does not finish with empty job structure.";
#else
    FAIL() << "# Unit does not finish with empty job structure.";

    int jobId{};

    // EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    // EXPECT_EQ(m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function,
    // nullptr), 0);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(&m_tp, &m_TPJob, nullptr),
              EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(&m_tp, &m_TPJob, &jobId), 0);
#endif
}

TEST_F(ThreadPoolTestSuite,
       add_persistent_job_to_threadpool_with_initialized_job) {
    int jobId{};

    // Initialize a job
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    EXPECT_EQ(
        m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function, nullptr), 0);
    // process unit
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(&m_tp, &m_TPJob, &jobId), 0);
    EXPECT_EQ(jobId, 0);
}

TEST_F(ThreadPoolTestSuite, add_persistent_job_to_empty_threadpool) {
    int jobId{};

    // process unit
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(nullptr, nullptr, nullptr),
              EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(nullptr, nullptr, &jobId),
              EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(nullptr, &m_TPJob, nullptr),
              EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(nullptr, &m_TPJob, &jobId),
              EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(&m_tp, nullptr, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAddPersistent(&m_tp, nullptr, &jobId), EINVAL);
    EXPECT_EQ(jobId, 0);
}

TEST_F(ThreadPoolTestSuite, get_current_attributes_of_threadpool) {
    // set default attributes
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    // process unit
    EXPECT_EQ(m_tpObj.ThreadPoolGetAttr(nullptr, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetAttr(nullptr, &m_TPAttr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetAttr(&m_tp, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetAttr(&m_tp, &m_TPAttr), 0);

    EXPECT_EQ(m_TPAttr.minThreads, 1);
    EXPECT_EQ(m_TPAttr.maxThreads, 10);
    EXPECT_EQ(m_TPAttr.stackSize, 0);
    EXPECT_EQ(m_TPAttr.maxIdleTime, 10000);
    EXPECT_EQ(m_TPAttr.jobsPerThread, 10);
    EXPECT_EQ(m_TPAttr.maxJobsTotal, 100);
    EXPECT_EQ(m_TPAttr.starvationTime, 500);
    // Default scheduling policy is OS dependent
#ifdef __APPLE__
    EXPECT_EQ(m_TPAttr.schedPolicy, 1);
#else
    EXPECT_EQ(m_TPAttr.schedPolicy, SCHED_OTHER);
#endif
}

TEST_F(ThreadPoolTestSuite, set_threadpool_attributes) {
#ifdef __APPLE__
    // This test sometimes may result in segfaults on other tests on macOS It
    // is not reproducable. Seems to be a problem with resources on the test
    // environment.
    GTEST_SKIP() << "  BUG! Side effects by failing other tests if running "
                    "this test on macOS.\n";
#else
    std::cout << "  BUG! Side effects by failing other tests if running this "
                 "test on macOS.\n";
#endif // __APPLE__

    // Check edge conditions
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(nullptr, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(nullptr, &m_TPAttr), EINVAL);
#ifdef OLD_TEST
    std::cout << "  BUG! SEGFAULT with uninitialized threadpool on different "
                 "OS, should fail with error return.\n";
    // EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, nullptr), EINVAL);
    // EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, &m_TPAttr), 0);
#else  // OLD_TEST
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, nullptr), EINVAL)
        << "# SEGFAULT with uninitialized threadpool on different OS, should "
           "fail with error return.";
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, &m_TPAttr), 0)
        << "# SEGFAULT with uninitialized threadpool on different OS, should "
           "fail with error return.";
#endif // OLD_TEST
    // initialize threadpool with default attributes
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    // points to empty attribute structure
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, &m_TPAttr), 0);
    // set default attributes again
    EXPECT_EQ(m_tpObj.ThreadPoolSetAttr(&m_tp, nullptr), 0);
}

TEST_F(ThreadPoolTestSuite, add_job_with_edge_conditions_to_threadpool) {
    int jobId{};

    EXPECT_EQ(m_tpObj.ThreadPoolAdd(nullptr, nullptr, &jobId), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(nullptr, &m_TPJob, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(nullptr, &m_TPJob, &jobId), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, nullptr, nullptr), EINVAL);
#ifdef OLD_TEST
    // EXPECT_EQ(m_tpObj.ThreadPoolAdd(nullptr, nullptr, nullptr), EINVAL);
    // EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, nullptr, &jobId), EINVAL);
    // EXPECT_EQ(m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function,
    // nullptr), 0);
    std::cout << "  BUG! SEGFAULTs on macOS.\n";
    // EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, &m_TPJob, nullptr), EOUTOFMEM);
    // EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, &m_TPJob, &jobId), EOUTOFMEM);
    std::cout << "  BUG! There should not be an error message. O jobs are not "
                 "too many jobs.\n";
#else
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(nullptr, nullptr, nullptr), EINVAL)
        << "# SEGFAULT on macOS.";
    EXPECT_EQ(m_tpObj.ThreadPoolAdd(&m_tp, nullptr, &jobId), EINVAL)
        << "# SEGFAULT on macOS.";
    EXPECT_EQ(
        m_tpObj.TPJobInit(&m_TPJob, (start_routine)start_function, nullptr), 0)
        << "# SEGFAULT on macOS.";
    EXPECT_NE(m_tpObj.ThreadPoolAdd(&m_tp, &m_TPJob, nullptr), EOUTOFMEM)
        << "# There should not be an error message. O jobs are not too many "
           "jobs.";
    EXPECT_NE(m_tpObj.ThreadPoolAdd(&m_tp, &m_TPJob, &jobId), EOUTOFMEM)
        << "# There should not be an error message. O jobs are not too many "
           "jobs.";
#endif
}

TEST_F(ThreadPoolTestSuite, remove_job_with_edge_conditions_from_threadpool) {
    ThreadPoolJob removedJob{};

    EXPECT_EQ(m_tpObj.ThreadPoolRemove(nullptr, 0, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolRemove(nullptr, 0, &removedJob), EINVAL);

    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);

    EXPECT_EQ(m_tpObj.ThreadPoolRemove(&m_tp, 0, nullptr), INVALID_JOB_ID);
    EXPECT_EQ(m_tpObj.ThreadPoolRemove(&m_tp, 0, &removedJob), INVALID_JOB_ID);
}

TEST_F(ThreadPoolTestSuite, get_and_print_threadpool_status) {
    ThreadPoolStats stats{};

    // Initialize threadpool
    EXPECT_EQ(m_tpObj.ThreadPoolInit(&m_tp, nullptr), 0);
    // Get status
    EXPECT_EQ(m_tpObj.ThreadPoolGetStats(nullptr, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetStats(nullptr, &stats), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetStats(&m_tp, nullptr), EINVAL);
    EXPECT_EQ(m_tpObj.ThreadPoolGetStats(&m_tp, &stats), 0);
    // Print status
    m_tpObj.ThreadPoolPrintStats(&stats);
}

TEST_F(ThreadPoolTestSuite, gettimeofday) {
    timeval tv{};

    EXPECT_EQ(m_tpObj.gettimeofday(&tv, nullptr), 0);
    EXPECT_GT(tv.tv_sec, 1635672176); // that is about 2021-10-31T10:24
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
