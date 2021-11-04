// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-03

// Note
// -------------
// Testing POSIX Threads may result in undefined edge conditions resulting in
// segmentation faults. So for these tests it has been taken additional effort
// to isolate the tests and simplify them to ensure stability. Only simple tests
// without fixtures have been used. All variables and objects have only local
// scope to ensure that they are destroyed with finishing the particular test.
// To isolate error conditions from normal execution two test suits are used:
// TEST(ThreadPoolNormalTestSuite) and TEST(ThreadPoolErrorCondTestSuite). No
// mocks are used. I have found so far that you always have to use
// ThreadPoolShutdown() after using ThreadPoolInit(). Otherwise you will see
// random segfaults after several successful program executions (after about 200
// to 6000 times). You can provoke this with test repetition, e.g.:
// ./gtests/build/test_ThreadPool_old  --gtest_brief=1 --gtest_repeat=10000
// --gtest_filter=ThreadPoolNormalTestSuite.init_and_shutdown_threadpool

#include "gtest/gtest.h"

#include "FreeList.cpp"
#include "LinkedList.cpp"

#include "ThreadPool.cpp"
#include "ThreadPool.hpp"

namespace upnp {

//
//###############################
// ThreadPool Testsuite         #
//###############################

// This is a dummy start routine for a threadpool job
start_routine start_function() { return 0; }

//
TEST(ThreadPoolNormalTestSuite, init_and_shutdown_threadpool) {
    CThreadPool tpObj{}; // ThreadPool Object
    ThreadPool tp{};     // Structure for a threadpool

    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! Finish a test without threadpool shutdown should be "
                 "possible without random segfaults.\n";
#else
    GTEST_FAIL() << "# Finish a test without threadpool shutdown should be "
                    "possible without random segfaults.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, init_and_shutdown_threadpool) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.ThreadPoolInit(nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolShutdown(nullptr), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, add_and_remove_job_on_threadpool) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPool tp{};       // Structure for a threadpool
    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    int jobId{};
    ThreadPoolJob removedJob{};

    // Initialize threadpool and job
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);
    // Add and remove job
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, &TPJob, &jobId), 0);
    EXPECT_EQ(jobId, 0);
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, jobId, &removedJob), 0);
    EXPECT_EQ(removedJob.jobId, jobId);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolNormalTestSuite, add_and_remove_3_jobs_on_threadpool) {
    CThreadPool tpObj{};    // ThreadPool Object
    ThreadPool tp{};        // Structure for a threadpool
    ThreadPoolJob TPJob0{}; // Structure for a threadpool job
    ThreadPoolJob TPJob1{};
    ThreadPoolJob TPJob2{};
    int jobId0{};
    int jobId1{};
    int jobId2{};
    ThreadPoolJob removedJob0{};
    ThreadPoolJob removedJob1{};
    ThreadPoolJob removedJob2{};

    // Initialize threadpool and job
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob0, (start_routine)start_function, nullptr),
              0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob1, (start_routine)start_function, nullptr),
              0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob2, (start_routine)start_function, nullptr),
              0);

    // Add jobs
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, &TPJob0, &jobId0), 0);
    EXPECT_EQ(jobId0, 0);
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, &TPJob1, &jobId1), 0);
    EXPECT_EQ(jobId1, 1);
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, &TPJob2, &jobId2), 0);
    EXPECT_EQ(jobId2, 2);

    // Remove second, first, last job
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, jobId1, &removedJob1), 0);
    EXPECT_EQ(removedJob1.jobId, 1);
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, jobId0, &removedJob0), 0);
    EXPECT_EQ(removedJob0.jobId, 0);
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, jobId2, &removedJob2), 0);
    EXPECT_EQ(removedJob2.jobId, 2);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, add_job_to_threadpool) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPool tp{};       // Structure for a threadpool
    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    int jobId{};

    // Initialize threadpool and job
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);

    EXPECT_EQ(tpObj.ThreadPoolAdd(nullptr, nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAdd(nullptr, nullptr, &jobId), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAdd(nullptr, &TPJob, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAdd(nullptr, &TPJob, &jobId), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAdd(&tp, nullptr, &jobId), EINVAL);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, remove_job_from_threadpool) {
    CThreadPool tpObj{}; // ThreadPool Object
    ThreadPool tp{};     // Structure for a threadpool
    ThreadPoolJob removedJob{};

    EXPECT_EQ(tpObj.ThreadPoolRemove(nullptr, 0, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolRemove(nullptr, 0, &removedJob), EINVAL);

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // Remove a not existing/added job
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, 0, nullptr), INVALID_JOB_ID);
    EXPECT_EQ(tpObj.ThreadPoolRemove(&tp, 0, &removedJob), INVALID_JOB_ID);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolNormalTestSuite, init_job_and_set_job_priority) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPoolJob TPJob{}; // Structure for a threadpool job

    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);

    EXPECT_EQ(tpObj.TPJobSetPriority(&TPJob, LOW_PRIORITY), 0);
    EXPECT_EQ(tpObj.TPJobSetPriority(&TPJob, MED_PRIORITY), 0);
    EXPECT_EQ(tpObj.TPJobSetPriority(&TPJob, HIGH_PRIORITY), 0);
}

TEST(ThreadPoolErrorCondTestSuite, init_job_and_set_job_priority) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPoolJob TPJob{}; // Structure for a threadpool job

    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);

    EXPECT_EQ(tpObj.TPJobSetPriority(nullptr, (ThreadPriority)-1), EINVAL);
    EXPECT_EQ(tpObj.TPJobSetPriority(nullptr, LOW_PRIORITY), EINVAL);
    EXPECT_EQ(tpObj.TPJobSetPriority(&TPJob, (ThreadPriority)-1), EINVAL);
    EXPECT_EQ(tpObj.TPJobSetPriority(&TPJob, (ThreadPriority)3), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_job_free_function) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPoolJob TPJob{}; // Structure for a threadpool job

    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);
    EXPECT_EQ(tpObj.TPJobSetFreeFunction(&TPJob, DEFAULT_FREE_ROUTINE), 0);
}

TEST(ThreadPoolErrorCondTestSuite, set_job_free_function) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPoolJob TPJob{}; // Structure for a threadpool job

    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);
    EXPECT_EQ(tpObj.TPJobSetFreeFunction(nullptr, DEFAULT_FREE_ROUTINE),
              EINVAL);
}

TEST(ThreadPoolNormalTestSuite, init_threadpool_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrInit(&TPAttr), 0);
}

TEST(ThreadPoolErrorCondTestSuite, init_threadpool_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrInit(nullptr), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_maximal_threads_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, 0), 0);
    EXPECT_EQ(TPAttr.minThreads, 0);
    EXPECT_EQ(TPAttr.maxThreads, 0);

    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, 1), 0);
    EXPECT_EQ(TPAttr.minThreads, 0);
    EXPECT_EQ(TPAttr.maxThreads, 1);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set maxThreads < 0"
              << " or < minThreads.\n";
    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, -1), 0);
    TPAttr.minThreads = 2;
    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, 1), 0);
#else
    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set maxThreads < 0.";
    TPAttr.minThreads = 2;
    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(&TPAttr, 1), EINVAL)
        << "# It should not be possible to set maxThreads < minThreads.";
#endif
    EXPECT_EQ(TPAttr.minThreads, 2);
    EXPECT_EQ(TPAttr.maxThreads, 1);
}

TEST(ThreadPoolErrorCondTestSuite, set_maximal_threads_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetMaxThreads(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_minimal_threads_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, 0), 0);
    EXPECT_EQ(TPAttr.minThreads, 0);
    EXPECT_EQ(TPAttr.maxThreads, 0);

    TPAttr.maxThreads = 2;
    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, 1), 0);
    EXPECT_EQ(TPAttr.minThreads, 1);
    EXPECT_EQ(TPAttr.maxThreads, 2);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set minThreads < 0"
              << " or > maxThreads.\n";
    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, -1), 0);
    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, 3), 0);
    EXPECT_EQ(TPAttr.minThreads, 3);
#else
    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set minThreads < 0.";
    EXPECT_EQ(tpObj.TPAttrSetMinThreads(&TPAttr, 3), EINVAL)
        << "# It should not be possible to set minThreads > maxThreads.";
    EXPECT_EQ(TPAttr.minThreads, 1)
        << "# Wrong settings should not modify old minThreads value.";
#endif
    EXPECT_EQ(TPAttr.maxThreads, 2);
}

TEST(ThreadPoolErrorCondTestSuite, set_minimal_threads_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetMinThreads(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_stack_size_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetStackSize(&TPAttr, 0), 0);
    EXPECT_EQ(tpObj.TPAttrSetStackSize(&TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set StackSize < 0.\n";
    EXPECT_EQ(tpObj.TPAttrSetStackSize(&TPAttr, -1), 0);
    EXPECT_EQ(TPAttr.stackSize, -1);
#else
    EXPECT_EQ(tpObj.TPAttrSetStackSize(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set StackSize < 0.";
    EXPECT_EQ(TPAttr.stackSize, 1)
        << "# Wrong settings should not modify old stackSize value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_stack_size_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetStackSize(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_idle_time_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetIdleTime(&TPAttr, 0), 0);
    EXPECT_EQ(tpObj.TPAttrSetIdleTime(&TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set IdleTime < 0.\n";
    EXPECT_EQ(tpObj.TPAttrSetIdleTime(&TPAttr, -1), 0);
    EXPECT_EQ(TPAttr.maxIdleTime, -1);
#else
    EXPECT_EQ(tpObj.TPAttrSetIdleTime(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set IdleTime < 0.";
    EXPECT_EQ(TPAttr.maxIdleTime, 1)
        << "# Wrong settings should not modify old maxIdleTime value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_idle_time_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetIdleTime(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_jobs_per_thread_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetJobsPerThread(&TPAttr, 0), 0);
    EXPECT_EQ(tpObj.TPAttrSetJobsPerThread(&TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set JobsPerThread < 0.\n";
    EXPECT_EQ(tpObj.TPAttrSetJobsPerThread(&TPAttr, -1), 0);
    EXPECT_EQ(TPAttr.jobsPerThread, -1);
#else
    EXPECT_EQ(tpObj.TPAttrSetJobsPerThread(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set JobsPerThread < 0.";
    EXPECT_EQ(TPAttr.jobsPerThread, 1)
        << "# Wrong settings should not modify old JobsPerThread value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_jobs_per_thread_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetJobsPerThread(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_starvation_time_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetStarvationTime(&TPAttr, 0), 0);
    EXPECT_EQ(tpObj.TPAttrSetStarvationTime(&TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout
        << "  BUG! It should not be possible to set StarvationTime < 0.\n";
    EXPECT_EQ(tpObj.TPAttrSetStarvationTime(&TPAttr, -1), 0);
    EXPECT_EQ(TPAttr.starvationTime, -1);
#else
    EXPECT_EQ(tpObj.TPAttrSetStarvationTime(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set StarvationTime < 0.";
    EXPECT_EQ(TPAttr.starvationTime, 1)
        << "# Wrong settings should not modify old starvationTime value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_starvation_time_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetStarvationTime(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_scheduling_policy_to_attribute) {
    // std::cout << "SCHED_OTHER: " << SCHED_OTHER
    //     << ", SCHED_IDLE: " << SCHED_IDLE
    //     << ", SCHED_BATCH: " << SCHED_BATCH << ", SCHED_FIFO: " << SCHED_FIFO
    //     << ", SCHED_RR: " << SCHED_RR
    //     << ", SCHED_DEADLINE; " << SCHED_DEADLINE << "\n";

    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetSchedPolicy(&TPAttr, SCHED_OTHER), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! Only SCHED_OTHER, SCHED_IDLE, SCHED_BATCH, SCHED_FIFO,"
              << " SCHED_RR or SCHED_DEADLINE should be valid.\n";
    EXPECT_EQ(tpObj.TPAttrSetSchedPolicy(&TPAttr, 0x5a5a), 0);
    EXPECT_EQ(TPAttr.schedPolicy, 0x5a5a);
#else
    EXPECT_EQ(tpObj.TPAttrSetSchedPolicy(&TPAttr, 0x5a5a), EINVAL)
        << "# Only SCHED_OTHER, SCHED_IDLE, SCHED_BATCH, SCHED_FIFO,"
        << " SCHED_RR or SCHED_DEADLINE should be valid.";
    EXPECT_EQ(TPAttr.schedPolicy, SCHED_OTHER)
        << "# Wrong settings should not modify old schedPolicy value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_scheduling_policy_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetSchedPolicy(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, set_max_jobs_qeued_totally_to_attribute) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.TPAttrSetMaxJobsTotal(&TPAttr, 0), 0);
    EXPECT_EQ(tpObj.TPAttrSetMaxJobsTotal(&TPAttr, 1), 0);
#ifdef OLD_TEST
    std::cout << "  BUG! It should not be possible to set MaxJobsTotal < 0.\n";
    EXPECT_EQ(tpObj.TPAttrSetMaxJobsTotal(&TPAttr, -1), 0);
    EXPECT_EQ(TPAttr.maxJobsTotal, -1);
#else
    EXPECT_EQ(tpObj.TPAttrSetMaxJobsTotal(&TPAttr, -1), EINVAL)
        << "# It should not be possible to set MaxJobsTotal < 0.";
    EXPECT_EQ(TPAttr.maxJobsTotal, 1)
        << "# Wrong settings should not modify old maxJobsTotal value.";
#endif
}

TEST(ThreadPoolErrorCondTestSuite, set_max_jobs_qeued_totally_to_attribute) {
    CThreadPool tpObj{}; // ThreadPool Object

    EXPECT_EQ(tpObj.TPAttrSetMaxJobsTotal(nullptr, 0), EINVAL);
}

TEST(ThreadPoolNormalTestSuite,
     add_persistent_job_to_threadpool_with_initialized_job) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPool tp{};       // Structure for a threadpool
    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    int jobId = -1;

    // Initialize threadpool and job
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);
    // Add persitent job
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(&tp, &TPJob, &jobId), 0);
    EXPECT_EQ(jobId, 0);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolNormalTestSuite,
     add_persistent_job_to_threadpool_with_no_jobId_returned) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPool tp{};       // Structure for a threadpool
    ThreadPoolJob TPJob{}; // Structure for a threadpool job

    // Initialize threadpool and job
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)start_function, nullptr),
              0);
    // Add persitent job
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(&tp, &TPJob, nullptr), 0);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, add_persistent_job_to_empty_threadpool) {
    CThreadPool tpObj{};   // ThreadPool Object
    ThreadPool tp{};       // Structure for a threadpool
    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    int jobId = -1;

    // process unit
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(nullptr, nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(nullptr, nullptr, &jobId), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(nullptr, &TPJob, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(nullptr, &TPJob, &jobId), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(&tp, nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolAddPersistent(&tp, nullptr, &jobId), EINVAL);
    EXPECT_EQ(jobId, -1);
}

TEST(ThreadPoolNormalTestSuite, get_current_attributes_of_threadpool) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    // Initialize threadpool with default attributes
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // Process unit
    EXPECT_EQ(tpObj.ThreadPoolGetAttr(&tp, &TPAttr), 0);

    EXPECT_EQ(TPAttr.minThreads, 1);
    EXPECT_EQ(TPAttr.maxThreads, 10);
    EXPECT_EQ(TPAttr.stackSize, 0);
    EXPECT_EQ(TPAttr.maxIdleTime, 10000);
    EXPECT_EQ(TPAttr.jobsPerThread, 10);
    EXPECT_EQ(TPAttr.maxJobsTotal, 100);
    EXPECT_EQ(TPAttr.starvationTime, 500);
    // Default scheduling policy is OS dependent
#ifdef __APPLE__
    EXPECT_EQ(TPAttr.schedPolicy, 1);
#else
    EXPECT_EQ(TPAttr.schedPolicy, SCHED_OTHER);
#endif
    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, get_current_attributes_of_threadpool) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    // Initialize threadpool with default attributes
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // Process unit
    EXPECT_EQ(tpObj.ThreadPoolGetAttr(nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolGetAttr(nullptr, &TPAttr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolGetAttr(&tp, nullptr), EINVAL);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolNormalTestSuite, set_threadpool_attributes) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    // initialize threadpool with default attributes
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // points to empty attribute structure
    EXPECT_EQ(tpObj.ThreadPoolSetAttr(&tp, &TPAttr), 0);
    // Check attributes
    EXPECT_EQ(tp.attr.minThreads, 0);
    EXPECT_EQ(tp.attr.maxThreads, 0);
    EXPECT_EQ(tp.attr.stackSize, 0);
    EXPECT_EQ(tp.attr.maxIdleTime, 0);
    EXPECT_EQ(tp.attr.jobsPerThread, 0);
    EXPECT_EQ(tp.attr.maxJobsTotal, 0);
    EXPECT_EQ(tp.attr.starvationTime, 0);
    EXPECT_EQ(tp.attr.schedPolicy, 0);

    // set default attributes
    EXPECT_EQ(tpObj.ThreadPoolSetAttr(&tp, nullptr), 0);
    EXPECT_EQ(tp.attr.minThreads, 1);
    EXPECT_EQ(tp.attr.maxThreads, 10);
    EXPECT_EQ(tp.attr.stackSize, 0);
    EXPECT_EQ(tp.attr.maxIdleTime, 10000);
    EXPECT_EQ(tp.attr.jobsPerThread, 10);
    EXPECT_EQ(tp.attr.maxJobsTotal, 100);
    EXPECT_EQ(tp.attr.starvationTime, 500);
    // Default scheduling policy is OS dependent
#ifdef __APPLE__
    EXPECT_EQ(tp.attr.schedPolicy, 1);
#else
    EXPECT_EQ(tp.attr.schedPolicy, SCHED_OTHER);
#endif
    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, set_threadpool_attributes) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPoolAttr TPAttr{}; // Structure for a threadpool attribute

    EXPECT_EQ(tpObj.ThreadPoolSetAttr(nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolSetAttr(nullptr, &TPAttr), EINVAL);
}

TEST(ThreadPoolNormalTestSuite, get_and_print_threadpool_status) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolStats stats{}; // Structure for the threadpool status

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, &stats), 0);
    // Print status
    tpObj.ThreadPoolPrintStats(&stats);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolErrorCondTestSuite, get_and_print_threadpool_status) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolStats stats{}; // Structure for the threadpool status

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(nullptr, nullptr), EINVAL);
    EXPECT_EQ(tpObj.ThreadPoolGetStats(nullptr, &stats), EINVAL);

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, nullptr), EINVAL);

    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolNormalTestSuite, gettimeofday) {
    CThreadPool tpObj{}; // ThreadPool Object
    timeval tv{};

    EXPECT_EQ(tpObj.gettimeofday(&tv, nullptr), 0);
    EXPECT_GT(tv.tv_sec, 1635672176); // that is about 2021-10-31T10:24
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
