// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-06

// Note
// -------------
// Testing POSIX Threads may result in undefined edge conditions resulting in
// segmentation faults. So for these tests it has been taken additional effort
// to isolate the tests and simplify them to ensure stability. Only simple tests
// without fixtures have been used. All variables and objects have only local
// scope to ensure that they are destroyed with finishing the particular test.
// To isolate error conditions from normal execution two test suits are used:
// TEST(TimerThreadNormalTestSuite) and TEST(TimerThreadErrorCondTestSuite). No
// mocks are used. I have found so far that you always have to use
// ThreadPoolShutdown() after using ThreadPoolInit(). Otherwise you will see
// random segfaults after several successful program executions (after about 10
// to 6000 times). You can provoke this with test repetition, e.g.:
// ./utest/build/test_TimerThread_old  --gtest_brief=1 --gtest_repeat=10000
// --gtest_filter=TimerThreadNormalTestSuite.init_and_shutdown_timerthread
// --Ingo

#include <TimerThread.hpp>
#include <pupnp/ThreadPool.hpp>

#include <chrono>
#include <thread>

#include <upnplib/general.hpp>

#include <utest/utest.hpp>


namespace utest {

using ::pupnp::CThreadPool;


//###############################
// TimerThread Interface        #
//###############################

class ITimerThread {
  public:
    virtual ~ITimerThread() {}
    virtual int TimerThreadInit(TimerThread* timer, ThreadPool* tp) = 0;
    virtual int TimerThreadSchedule(TimerThread* timer, time_t timeout,
                                    TimeoutType type, ThreadPoolJob* job,
                                    Duration duration, int* id) = 0;
    virtual int TimerThreadRemove(TimerThread* timer, int id,
                                  ThreadPoolJob* out) = 0;
    virtual int TimerThreadShutdown(TimerThread* timer) = 0;
};

class CTimerThread : public ITimerThread {
  public:
    virtual ~CTimerThread() {}

    int TimerThreadInit(TimerThread* timer, ThreadPool* tp) override {
        return ::TimerThreadInit(timer, tp);
    }
    int TimerThreadSchedule(TimerThread* timer, time_t timeout,
                            TimeoutType type, ThreadPoolJob* job,
                            Duration duration, int* id) override {
        return ::TimerThreadSchedule(timer, timeout, type, job, duration, id);
    }
    int TimerThreadRemove(TimerThread* timer, int id,
                          ThreadPoolJob* out) override {
        return ::TimerThreadRemove(timer, id, out);
    }
    int TimerThreadShutdown(TimerThread* timer) override {
        return ::TimerThreadShutdown(timer);
    }
};


//###############################
// TimerThread Testsuite        #
//###############################

TEST(TimerThreadNormalTestSuite, init_and_shutdown_timerthread) {
    CThreadPool tpObj{};  // ThreadPool Object
    ThreadPool tp{};      // Structure for a threadpool
    CTimerThread tmObj{}; // TimerThread Object
    TimerThread timer{};

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    // Initialize timer thread
    EXPECT_EQ(tmObj.TimerThreadInit(&timer, &tp), 0);

    // Shutdown timer thread
    EXPECT_EQ(tmObj.TimerThreadShutdown(&timer), 0);
    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

// Start function for schedule_timer_thread
void start_function1([[maybe_unused]] void* arg) {
    std::cout << "Executed start_function1.\n";
}

TEST(TimerThreadNormalTestSuite, schedule_timer_thread) {
    // Because we have to wait some seconds to really test the scheduled thread
    // this test is very expensive and we cannot simply check the asynchronous
    // output of the start_function1. So this test isn't enabled on the normal
    // test suite but it shows how to schedule a timer thread. Testing this in
    // the production environment will be done with the upnplib info program.

    if (old_code)
        std::cout << CYEL "[ BUG      ]" CRES
                  << " Return value of the thread start routine should not be "
                     "ignored.\n";
    else
        GTEST_FAIL()
            << "  # Return value of the thread start routine should not "
               "be ignored.";

    GTEST_SKIP() << "This test is skipped because it is very expensive and "
                    "difficult to completely test.";

    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolStats stats{}; // Structure for the threadpool status

    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    // ThreadPoolJob removedJob{};

    CTimerThread tmObj{}; // TimerThread Object
    TimerThread timer{};
    int tmId = -1;

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);

    // Initialize threadpool job
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)&start_function1, nullptr),
              0);

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, &stats), 0);
    EXPECT_EQ(stats.persistentThreads, 0);
    EXPECT_EQ(stats.totalThreads, 1);
    // std::cout << "---- Status after initializing threadpool job ----\n";
    // tpObj.ThreadPoolPrintStats(&stats); // Print status

    // Initialize timer thread
    EXPECT_EQ(tmObj.TimerThreadInit(&timer, &tp), 0);

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, &stats), 0);
    EXPECT_EQ(stats.persistentThreads, 1);
    EXPECT_EQ(stats.totalThreads, 2);

    // Schedule timer thread running after 1 second
    EXPECT_EQ(tmObj.TimerThreadSchedule(&timer, 1, REL_SEC, &TPJob, SHORT_TERM,
                                        &tmId),
              0);
    EXPECT_EQ(tmId, 0);

    // Wait 2 seconds for the thread
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Shutdown timer thread, order is important!
    EXPECT_EQ(tmObj.TimerThreadShutdown(&timer), 0);
    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

// Start function for remove_timer_thread
void start_function2([[maybe_unused]] void* arg) {
    std::cout << "Executed start_function2. This should never "
                 "occur.\nTestprogram exit.\n";
    exit(EXIT_FAILURE);
}

TEST(TimerThreadNormalTestSuite, remove_timer_thread) {
    CThreadPool tpObj{};     // ThreadPool Object
    ThreadPool tp{};         // Structure for a threadpool
    ThreadPoolStats stats{}; // Structure for the threadpool status

    ThreadPoolJob TPJob{}; // Structure for a threadpool job
    // ThreadPoolJob removedJob{};

    CTimerThread tmObj{}; // TimerThread Object
    TimerThread timer{};
    int tmId = -1;

    // Initialize threadpool
    EXPECT_EQ(tpObj.ThreadPoolInit(&tp, nullptr), 0);
    // Initialize threadpool job
    EXPECT_EQ(tpObj.TPJobInit(&TPJob, (start_routine)&start_function2, nullptr),
              0);

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, &stats), 0);
    EXPECT_EQ(stats.persistentThreads, 0);
    EXPECT_EQ(stats.totalThreads, 1);
    // std::cout << "---- Status after initializing threadpool job ----\n";
    // tpObj.ThreadPoolPrintStats(&stats); // Print status

    // Initialize timer thread
    EXPECT_EQ(tmObj.TimerThreadInit(&timer, &tp), 0);

    // Get status
    EXPECT_EQ(tpObj.ThreadPoolGetStats(&tp, &stats), 0);
    EXPECT_EQ(stats.persistentThreads, 1);
    EXPECT_EQ(stats.totalThreads, 2);

    // Schedule timer thread running after 2 seconds
    EXPECT_EQ(tmObj.TimerThreadSchedule(&timer, 2, REL_SEC, &TPJob, SHORT_TERM,
                                        &tmId),
              0);
    EXPECT_EQ(tmId, 0);

    // Remove the scheduled thread before it was added to the threadpool for
    // execution.
    EXPECT_EQ(tmObj.TimerThreadRemove(&timer, tmId, &TPJob), 0);

    // Wait 3 seconds for the thread. It should not be executed.
    // std::this_thread::sleep_for(std::chrono::seconds(3));

    // Shutdown timer thread, order is important!
    EXPECT_EQ(tmObj.TimerThreadShutdown(&timer), 0);
    // Shutdown threadpool
    EXPECT_EQ(tpObj.ThreadPoolShutdown(&tp), 0);
}

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
