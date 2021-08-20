// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-08-20

#ifndef NDEBUG
#define NDEBUG
#endif

#include "gtest/gtest.h"

// Test of local functions, not linkable
#include "FreeList.cpp"
#include "LinkedList.cpp"
#include "ThreadPool.cpp"
#include "TimerThread.cpp"

// testsuite with fixtures
//------------------------
class ThreadPoolFixtureTestSuite : public ::testing::Test {
  protected:
    // for testing job free_function
    free_routine free_func() { return 0; }
};

// testsuites without fixtures
//----------------------------
TEST(FreeListTestSuite, initialize_FreeList_with_no_items) {
    FreeList free_list = {};

    EXPECT_EQ(FreeListInit(nullptr, 0, 0), EINVAL);
    EXPECT_EQ(FreeListInit(&free_list, 0, 0), 0);
    // TODO! Should return nullptr as failure because of 0 items possible
    EXPECT_NE(FreeListAlloc(&free_list), nullptr);
    // EXPECT_EQ(FreeListAlloc(&free_list), nullptr);

    EXPECT_EQ(FreeListDestroy(nullptr), EINVAL);
    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

TEST(FreeListTestSuite, allocate_item_in_FreeList) {
    int element = 999;
    FreeList free_list = {};

    // maxFreeListLength is 2
    EXPECT_EQ(FreeListInit(&free_list, sizeof(element), 3), 0);
    // allocate item
    EXPECT_NE(FreeListAlloc(&free_list), nullptr);
    EXPECT_NE(FreeListAlloc(&free_list), nullptr);
    EXPECT_NE(FreeListAlloc(&free_list), nullptr);
    // Exeeding maxFreeListLength should return nullptr?
    // EXPECT_EQ(FreeListAlloc(&free_list), nullptr);
    // EXPECT_NE(FreeListAlloc(&free_list), nullptr);

    EXPECT_EQ(FreeListFree(&free_list, &element), 0);
    EXPECT_EQ(FreeListFree(&free_list, &element), 0);
    // TODO! Check why the test abort with 'munmap_chunk(): invalid pointer'
    // when freeing same amount of elements as allocated
    // EXPECT_EQ(FreeListFree(&free_list, &element), 0);

    // Destroy aborted test after free list with 'free(): invalid pointer'
    // EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

TEST(LinkedListTestSuite, process_nodes_add_delete_point_to) {
    LinkedList ll = {};
    int item1 = 111;
    int item2 = 222;
    int item3 = 333;
    int item4 = 444;

    EXPECT_EQ(ListInit(nullptr, nullptr, nullptr), EINVAL);
    EXPECT_EQ(ListInit(&ll, nullptr, nullptr), 0);

    EXPECT_EQ(ListHead(&ll), nullptr);
    EXPECT_EQ(ListTail(&ll), nullptr);

    ListNode* node1 = ListAddTail(&ll, &item1);
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(*(int*)node1->item, 111);

    ListNode* node2 = ListAddHead(&ll, &item2);
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(*(int*)node2->item, 222);

    ListNode* node3 = ListAddAfter(&ll, &item3, node1);
    EXPECT_NE(node3, nullptr);
    EXPECT_EQ(*(int*)node3->item, 333);

    ListNode* node4 = ListAddBefore(&ll, &item4, node1);
    EXPECT_NE(node4, nullptr);
    EXPECT_EQ(*(int*)node4->item, 444);

    ListNode* node = (ListNode*)ListDelNode(&ll, node1, 0);
    EXPECT_NE(node, nullptr);

    node = (ListNode*)ListDelNode(&ll, node2, 1);
    EXPECT_NE(node, nullptr);

    EXPECT_EQ(ListHead(&ll), node4);
    EXPECT_EQ(ListTail(&ll), node3);

    EXPECT_EQ(ListNext(&ll, node4), node3);
    EXPECT_EQ(ListNext(&ll, node3), nullptr);

    EXPECT_EQ(ListPrev(&ll, node3), node4);
    EXPECT_EQ(ListPrev(&ll, node4), nullptr);

    EXPECT_EQ(ListFind(&ll, nullptr, &item3), node3);
    EXPECT_EQ(ListFind(&ll, nullptr, &item1), nullptr);

    // ATTENTION! Node to start from is excluded from search list
    EXPECT_EQ(ListFind(&ll, node4, &item4), nullptr);

    EXPECT_EQ(ListSize(&ll), 2);

    EXPECT_EQ(ListDestroy(&ll, 0), 0);
    EXPECT_EQ(ListDestroy(&ll, 1), 0);

    EXPECT_EQ(ListSize(&ll), 0);
    EXPECT_EQ(ListHead(&ll), nullptr);
    EXPECT_EQ(ListTail(&ll), nullptr);
}

TEST(ThreadPoolTestSuite, initialize_threadpool) {
    GTEST_SKIP() << "This test produces random segfault\n";

    ThreadPool tp = {};

    // process unit
    EXPECT_EQ(ThreadPoolInit(nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolInit(&tp, nullptr), 0);
}

TEST(ThreadPoolTestSuite, add_persistent_job_to_threadpool_bug) {
    GTEST_SKIP() << "Unit hang, does not finish with empty job structure";

    ThreadPool tp = {};
    ThreadPoolJob job = {};
    int jobId = 0;

    EXPECT_EQ(ThreadPoolAddPersistent(&tp, &job, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(&tp, &job, &jobId), EINVAL);
}

TEST(ThreadPoolTestSuite, add_persistent_job_to_empty_threadpool) {
    ThreadPool tp = {};
    ThreadPoolJob job = {};
    int jobId = 0;

    // process unit
    EXPECT_EQ(ThreadPoolAddPersistent(nullptr, nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(nullptr, nullptr, &jobId), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(nullptr, &job, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(nullptr, &job, &jobId), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(&tp, nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAddPersistent(&tp, nullptr, &jobId), EINVAL);
    EXPECT_EQ(jobId, 0);
}

TEST(ThreadPoolTestSuite, get_current_attributes_of_threadpool) {
    GTEST_SKIP() << "This test produces random segfault\n";

    ThreadPool tp = {};
    ThreadPoolAttr attr = {};

    // set default attributes
    EXPECT_EQ(ThreadPoolInit(&tp, nullptr), 0);
    // process unit
    EXPECT_EQ(ThreadPoolGetAttr(nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolGetAttr(nullptr, &attr), EINVAL);
    EXPECT_EQ(ThreadPoolGetAttr(&tp, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolGetAttr(&tp, &attr), 0);

    EXPECT_EQ(attr.minThreads, 1);
    EXPECT_EQ(attr.maxThreads, 10);
    EXPECT_EQ(attr.stackSize, 0);
    EXPECT_EQ(attr.maxIdleTime, 10000);
    EXPECT_EQ(attr.jobsPerThread, 10);
    EXPECT_EQ(attr.maxJobsTotal, 100);
    EXPECT_EQ(attr.starvationTime, 500);
    EXPECT_EQ(attr.schedPolicy, 0);
}

TEST(ThreadPoolTestSuite, set_threadpool_attributes) {
    GTEST_SKIP() << "This test produces random segfault\n";

    ThreadPool tp = {};
    ThreadPoolAttr attr = {};

    // set default attributs
    EXPECT_EQ(ThreadPoolSetAttr(&tp, nullptr), 0);
    EXPECT_EQ(ThreadPoolSetAttr(&tp, &attr), 0);
}

TEST(ThreadPoolTestSuite, add_job_to_threadpool) {
    ThreadPool tp = {};
    ThreadPoolJob job = {};
    int jobId = 0;

    EXPECT_EQ(ThreadPoolAdd(nullptr, nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(nullptr, nullptr, &jobId), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(nullptr, &job, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(nullptr, &job, &jobId), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(&tp, nullptr, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(&tp, nullptr, &jobId), EINVAL);
    EXPECT_EQ(ThreadPoolAdd(&tp, &job, nullptr), EOUTOFMEM);
    EXPECT_EQ(ThreadPoolAdd(&tp, &job, &jobId), EOUTOFMEM);

    GTEST_SKIP() << "Threadpool job must be initialized\n";

    // EXPECT_EQ(TPJobInit(&job, func, &arg), 0);
    EXPECT_EQ(ThreadPoolAdd(&tp, &job, &jobId), 0);
}

TEST(ThreadPoolTestSuite, remove_job_from_threadpool) {
    GTEST_SKIP() << "This test produces random segfault\n";

    ThreadPool tp = {};
    int jobId = 0;
    ThreadPoolJob removedJob = {};

    EXPECT_EQ(ThreadPoolRemove(nullptr, 0, nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolRemove(nullptr, 0, &removedJob), EINVAL);

    // ThreadPool must be initialized, otherwise we get a segfault
    EXPECT_EQ(ThreadPoolInit(&tp, nullptr), 0);

    EXPECT_EQ(ThreadPoolRemove(&tp, 0, nullptr), EOUTOFMEM);
    EXPECT_EQ(ThreadPoolRemove(&tp, 0, &removedJob), EOUTOFMEM);

    GTEST_SKIP()
        << "Threadpool job must be initialized, segfault should be fixed\n";

    EXPECT_EQ(ThreadPoolRemove(&tp, 0, &removedJob), 0);
}

TEST(ThreadPoolTestSuite, shutdown_threadpool) {
    ThreadPool tp = {};
    EXPECT_EQ(ThreadPoolInit(&tp, nullptr), 0);
    // process unit
    EXPECT_EQ(ThreadPoolShutdown(nullptr), EINVAL);
    EXPECT_EQ(ThreadPoolShutdown(&tp), 0);
}

TEST(ThreadPoolTestSuite, add_persistent_job_to_threadpool) {
    GTEST_SKIP() << "Unknown how to initialize a threadpool job";

    ThreadPool tp = {};
    ThreadPoolJob job = {};
    int jobId = 0;

    // TPJobInit(&job, func, arg);

    // process unit
    EXPECT_EQ(ThreadPoolAddPersistent(&tp, &job, &jobId), 0);
    EXPECT_EQ(jobId, 0);
}

TEST(ThreadPoolTestSuite, set_job_priority) {
    ThreadPoolJob job = {};

    EXPECT_EQ(TPJobSetPriority(nullptr, LOW_PRIORITY), EINVAL);
    EXPECT_EQ(TPJobSetPriority(&job, (ThreadPriority)-1), EINVAL);
    EXPECT_EQ(TPJobSetPriority(&job, LOW_PRIORITY), 0);
    EXPECT_EQ(TPJobSetPriority(&job, MED_PRIORITY), 0);
    EXPECT_EQ(TPJobSetPriority(&job, HIGH_PRIORITY), 0);
    EXPECT_EQ(TPJobSetPriority(&job, (ThreadPriority)3), EINVAL);
}

TEST_F(ThreadPoolFixtureTestSuite, set_job_free_function) {
    ThreadPoolJob job = {};

    EXPECT_EQ(TPJobSetFreeFunction(&job, free_func()), 0);
}

TEST(ThreadPoolTestSuite, initialize_default_threadpool_attributes) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrInit(nullptr), EINVAL);
    EXPECT_EQ(TPAttrInit(&attr), 0);

    EXPECT_EQ(attr.minThreads, 1);
    EXPECT_EQ(attr.maxThreads, 10);
    EXPECT_EQ(attr.stackSize, 0);
    EXPECT_EQ(attr.maxIdleTime, 10000);
    EXPECT_EQ(attr.jobsPerThread, 10);
    EXPECT_EQ(attr.maxJobsTotal, 100);
    EXPECT_EQ(attr.starvationTime, 500);
    EXPECT_EQ(attr.schedPolicy, 0);
}

TEST(ThreadPoolTestSuite, set_maximal_threads) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetMaxThreads(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMaxThreads(&attr, 0), 0);
    EXPECT_EQ(attr.minThreads, 0);
    EXPECT_EQ(attr.maxThreads, 0);

    EXPECT_EQ(TPAttrSetMaxThreads(&attr, 1), 0);
    EXPECT_EQ(attr.minThreads, 0);
    EXPECT_EQ(attr.maxThreads, 1);

    GTEST_SKIP()
        << "It should not be possible to set maxThreads < 0 or < minThreads";

    EXPECT_EQ(TPAttrSetMaxThreads(&attr, -1), EINVAL);
    attr.minThreads = 2;
    EXPECT_EQ(TPAttrSetMaxThreads(&attr, 1), EINVAL);
    EXPECT_EQ(attr.minThreads, 2);
    EXPECT_EQ(attr.maxThreads, 1);
}

TEST(ThreadPoolTestSuite, set_minimal_threads) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetMinThreads(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMinThreads(&attr, 0), 0);
    EXPECT_EQ(attr.minThreads, 0);
    EXPECT_EQ(attr.maxThreads, 0);

    attr.maxThreads = 2;
    EXPECT_EQ(TPAttrSetMinThreads(&attr, 1), 0);
    EXPECT_EQ(attr.minThreads, 1);
    EXPECT_EQ(attr.maxThreads, 2);

    GTEST_SKIP()
        << "It should not be possible to set minThreads < 0 or > maxThreads";

    EXPECT_EQ(TPAttrSetMinThreads(&attr, -1), EINVAL);
    EXPECT_EQ(TPAttrSetMinThreads(&attr, 3), EINVAL);
    EXPECT_EQ(attr.minThreads, 1);
    EXPECT_EQ(attr.maxThreads, 2);
}

TEST(ThreadPoolTestSuite, set_stack_size) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetStackSize(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetStackSize(&attr, 0), 0);
    EXPECT_EQ(TPAttrSetStackSize(&attr, 1), 0);

    GTEST_SKIP() << "It should not be possible to set StackSize < 0";

    EXPECT_EQ(TPAttrSetStackSize(&attr, -1), EINVAL);
}

TEST(ThreadPoolTestSuite, set_idle_time) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetIdleTime(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetIdleTime(&attr, 0), 0);
    EXPECT_EQ(TPAttrSetIdleTime(&attr, 1), 0);

    GTEST_SKIP() << "It should not be possible to set IdleTime < 0";

    EXPECT_EQ(TPAttrSetStackSize(&attr, -1), EINVAL);
}

TEST(ThreadPoolTestSuite, set_jobs_per_thread) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetJobsPerThread(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetJobsPerThread(&attr, 0), 0);
    EXPECT_EQ(TPAttrSetJobsPerThread(&attr, 1), 0);

    GTEST_SKIP() << "It should not be possible to set JobsPerThread < 0";

    EXPECT_EQ(TPAttrSetStackSize(&attr, -1), EINVAL);
}

TEST(ThreadPoolTestSuite, set_starvation_time) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetStarvationTime(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetStarvationTime(&attr, 0), 0);
    EXPECT_EQ(TPAttrSetStarvationTime(&attr, 1), 0);

    GTEST_SKIP() << "It should not be possible to set StarvationTime < 0";

    EXPECT_EQ(TPAttrSetStackSize(&attr, -1), EINVAL);
}

TEST(ThreadPoolTestSuite, set_scheduling_policy) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetSchedPolicy(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetSchedPolicy(&attr, SCHED_OTHER), 0);
}

TEST(ThreadPoolTestSuite, set_max_jobs_qeued_totally) {
    ThreadPoolAttr attr = {};

    EXPECT_EQ(TPAttrSetMaxJobsTotal(nullptr, 0), EINVAL);
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&attr, 0), 0);
    EXPECT_EQ(TPAttrSetMaxJobsTotal(&attr, 1), 0);

    GTEST_SKIP() << "It should not be possible to set MaxJobsTotal < 0";

    EXPECT_EQ(TPAttrSetStackSize(&attr, -1), EINVAL);
}

TEST(ThreadPoolTestSuite, DiffMillis) {
    // Very simple, 999 us ingnored
    struct timeval time1 = {};
    struct timeval time2 = {};
    EXPECT_EQ(DiffMillis(&time1, &time2), 0);

    time1.tv_usec = 999;
    EXPECT_EQ(DiffMillis(&time1, &time2), 0);

    time1.tv_usec = 2000;
    time2.tv_usec = 1000;
    EXPECT_EQ(DiffMillis(&time1, &time2), 1);
    EXPECT_EQ(DiffMillis(&time2, &time1), -1);

    time1.tv_sec = 2;
    time2.tv_sec = 1;
    EXPECT_EQ(DiffMillis(&time1, &time2), 1001);
    EXPECT_EQ(DiffMillis(&time2, &time1), -1001);
}

TEST(ThreadPoolTestSuite, StatsInit_for_thread_statistic) {
    ThreadPoolStats stats;
    stats.totalTimeHQ = -1;
    StatsInit(&stats);
    EXPECT_EQ(stats.totalTimeHQ, -1);
}

TEST(ThreadPoolTestSuite, StatsAccount) {
    // TODO! Enable thread statistics in ThreadPool.h #define STATS 1
    ThreadPool tp;
    tp.stats.totalJobsLQ = 1;
    tp.stats.totalTimeLQ = 2;
    // process unit
    StatsAccountLQ(&tp, 1);
    EXPECT_EQ(tp.stats.totalJobsLQ, 1);
    EXPECT_EQ(tp.stats.totalTimeLQ, 2);
    // ThreadPool tp;
    // tp.stats.totalJobsLQ = 1;
    // tp.stats.totalTimeLQ = 2;
    // StatsAccountLQ(&tp, 1);
    // EXPECT_EQ(tp.stats.totalJobsLQ, 2);
    // EXPECT_EQ(tp.stats.totalTimeLQ, 3);

    tp.stats.totalJobsMQ = 3;
    tp.stats.totalTimeMQ = 4;
    // process unit
    StatsAccountMQ(&tp, 1);
    EXPECT_EQ(tp.stats.totalJobsMQ, 3);
    EXPECT_EQ(tp.stats.totalTimeMQ, 4);

    tp.stats.totalJobsHQ = 5;
    tp.stats.totalTimeHQ = 6;
    // process unit
    StatsAccountHQ(&tp, 1);
    EXPECT_EQ(tp.stats.totalJobsHQ, 5);
    EXPECT_EQ(tp.stats.totalTimeHQ, 6);

    ThreadPoolJob job;
    job.requestTime.tv_sec = 3;
    // process unit
    CalcWaitTime(&tp, LOW_PRIORITY, &job);
    EXPECT_EQ(tp.stats.totalTimeLQ, 2);
    EXPECT_EQ(job.requestTime.tv_sec, 3);
    // process unit
    CalcWaitTime(&tp, MED_PRIORITY, &job);
    EXPECT_EQ(tp.stats.totalTimeMQ, 4);
    EXPECT_EQ(job.requestTime.tv_sec, 3);
    // process unit
    CalcWaitTime(&tp, HIGH_PRIORITY, &job);
    EXPECT_EQ(tp.stats.totalTimeHQ, 6);
    EXPECT_EQ(job.requestTime.tv_sec, 3);

    time_t time = -1;
    // process unit
    EXPECT_EQ(StatsTime(&time), 0);
    EXPECT_EQ(time, -1);
}

TEST(ThreadPoolTestSuite, compare_threadpool_job) {
    ThreadPoolJob job1 = {};
    ThreadPoolJob job2 = {};
    job1.jobId = -1;
    job2.jobId = -1;
    // process unit
    EXPECT_TRUE(CmpThreadPoolJob(&job1, &job2));

    job1.jobId = 0;
    EXPECT_FALSE(CmpThreadPoolJob(&job1, &job2));
}

TEST(ThreadPoolTestSuite, free_threadpool_job) {
    // TODO! Check if FreeListFree can be called direct
    ThreadPool tp = {};
    ThreadPoolJob tpj = {};
    tp.jobFreeList.freeListLength = -1;

    // process unit
    // TODO! Check why test abort with 'free(): invalid pointer'
    // FreeThreadPoolJob(&tp, &tpj);
    // EXPECT_EQ(tp.jobFreeList.freeListLength, 0);
}

TEST(ThreadPoolTestSuite, set_policy_type) {
    // TODO! Check different operating systems
    EXPECT_EQ(SetPolicyType(DEFAULT_POLICY), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
