#ifndef COMPA_THREADPOOL_HPP
#define COMPA_THREADPOOL_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-07-31
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
/*!
 * \file
 * \ingroup threadutil
 * \brief Manage a threadpool (for internal use only).
 *
 * Because this is for internal use, parameters are NOT checked for validity.
 * The caller must ensure valid parameters.
 */

#include "LinkedList.hpp"
#include <upnplib/port_sock.hpp>

#if defined(_WIN32) || defined(DOXYGEN_RUN)
#if !defined(_TIMEZONE_DEFINED) || defined(DOXYGEN_RUN)
/// \brief Timezone
struct timezone {
    /*! \brief Minutes W of Greenwich */
    int tz_minuteswest;
    /*! \brief Type of dst correction */
    int tz_dsttime;
};
/// \brief Get time of day
int gettimeofday(struct timeval* tv, struct timezone* tz);
#endif
#else                 // _WIN32
/// \cond
#include <sys/time.h> /* for gettimeofday() */
/// \endcond
#endif                // _WIN32

/*! \brief Invalid JOB Id */
#define INVALID_JOB_ID (-2 & 1 << 29)

/*! Function for freeing a thread argument. */
typedef void (*free_routine)(void* arg);

/// \brief Duration
enum Duration { SHORT_TERM, PERSISTENT };

/// \brief Thread priority.
enum ThreadPriority { LOW_PRIORITY, MED_PRIORITY, HIGH_PRIORITY };

/*! default priority used by TPJobInit */
constexpr ThreadPriority DEFAULT_PRIORITY{MED_PRIORITY};

/*! default minimum used by TPAttrInit */
constexpr int DEFAULT_MIN_THREADS{1};

/*! default max used by TPAttrInit */
constexpr int DEFAULT_MAX_THREADS{10};

/*! default stack size used by TPAttrInit */
constexpr int DEFAULT_STACK_SIZE{0u};

/*! default jobs per thread used by TPAttrInit */
constexpr int DEFAULT_JOBS_PER_THREAD{10};

/*! default starvation time used by TPAttrInit */
constexpr int DEFAULT_STARVATION_TIME{500};

/*! default idle time used by TPAttrInit */
constexpr int DEFAULT_IDLE_TIME{10 * 1000};

/*! default free routine used TPJobInit */
constexpr free_routine DEFAULT_FREE_ROUTINE{nullptr};

/*! default max jobs used TPAttrInit */
constexpr int DEFAULT_MAX_JOBS_TOTAL{100};

/*!
 * \brief Statistics.
 *
 * Always include stats because code change is minimal.
 */
#define STATS 1

#ifdef _DEBUG
#define DEBUG 1
#endif

/// \brief Type of the thread policy.
typedef int PolicyType;

/// \brief Define default schedule policy that are defined in <sched.h>.
#define DEFAULT_POLICY SCHED_OTHER

/*! \brief Attributes for thread pool.
 *
 * Used to set and change parameters of thread pool. */
struct ThreadPoolAttr {
    /*! \brief ThreadPool will always maintain at least this many threads. */
    int minThreads;
    /*! \brief ThreadPool will never have more than this number of threads. */
    int maxThreads;
    /*! \brief This is the minimum stack size allocated for each thread. */
    size_t stackSize;
    /*! \brief this is the maximum time a thread will remain idle before dying
     * (in milliseconds). */
    int maxIdleTime;
    /*! \brief Jobs per thread to maintain. */
    int jobsPerThread;
    /*! \brief Maximum number of jobs that can be queued totally. */
    int maxJobsTotal;
    /*! \brief The time a low priority or med priority job waits before getting
     * bumped up a priority (in milliseconds). */
    int starvationTime;
    /*! \brief Scheduling policy to use. */
    PolicyType schedPolicy;
};

/*! \brief Internal ThreadPool Job. */
struct ThreadPoolJob {
    start_routine func;
    void* arg;
    free_routine free_func;
    struct timeval requestTime;
    ThreadPriority priority;
    int jobId;
};

/*! \brief Structure to hold statistics. */
struct ThreadPoolStats {
    double totalTimeHQ;
    int totalJobsHQ;
    double avgWaitHQ;
    double totalTimeMQ;
    int totalJobsMQ;
    double avgWaitMQ;
    double totalTimeLQ;
    int totalJobsLQ;
    double avgWaitLQ;
    double totalWorkTime;
    double totalIdleTime;
    int workerThreads;
    int idleThreads;
    int persistentThreads;
    int totalThreads;
    int maxThreads;
    int currentJobsHQ;
    int currentJobsLQ;
    int currentJobsMQ;
};

/*!
 * \brief A thread pool.
 *
 * Allows jobs to be scheduled for running by threads in a thread pool. The
 * thread pool is initialized with a minimum and maximum thread number as well
 * as a max idle time and a jobs per thread ratio. If a worker thread waits the
 * whole max idle time without receiving a job and the thread pool currently has
 * more threads running than the minimum then the worker thread will exit. If
 * when scheduling a job the current job to thread ratio becomes greater than
 * the set ratio and the thread pool currently has less than the maximum threads
 * then a new thread will be created.
 */
struct ThreadPool {
    /*! Mutex to protect job qs. */
    ithread_mutex_t mutex;
    /*! Condition variable to signal Q. */
    ithread_cond_t condition;
    /*! Condition variable for start and stop. */
    ithread_cond_t start_and_shutdown;
    /*! ids for jobs */
    int lastJobId;
    /*! whether or not we are shutting down */
    int shutdown;
    /*! total number of threads */
    int totalThreads;
    /*! flag that's set when waiting for a new worker thread to start */
    int pendingWorkerThreadStart;
    /*! number of threads that are currently executing jobs */
    int busyThreads;
    /*! number of persistent threads */
    int persistentThreads;
    /*! free list of jobs */
    FreeList jobFreeList;
    /*! low priority job Q */
    LinkedList lowJobQ;
    /*! med priority job Q */
    LinkedList medJobQ;
    /*! high priority job Q */
    LinkedList highJobQ;
    /*! persistent job */
    ThreadPoolJob* persistentJob;
    /*! thread pool attributes */
    ThreadPoolAttr attr;
    /*! statistics */
    ThreadPoolStats stats;
};

/*!
 * \brief Initializes and starts ThreadPool.
 *
 * Must be called first and only once for ThreadPool.
 *
 * \returns
 *  On success: **0**\n
 *  On error:
 *  - EINVAL with invalid ThreadPool.
 *  - EAGAIN if not enough system resources to create minimum threads.
 *  - INVALID_POLICY if schedPolicy can't be set.
 *  - EMAXTHREADS if minimum threads is greater than maximum threads.
 */
int ThreadPoolInit(
    /*! [in] Must be valid, non null, pointer to ThreadPool. */
    ThreadPool* tp,
    /*! [in] Can be nullptr. If not nullptr then attr contains the following
     * fields:
     *  - minWorkerThreads - minimum number of worker threads thread pool will
     *                       never have less than this number of threads.
     *  - maxWorkerThreads - maximum number of worker threads thread pool will
     *                       never have more than this number of threads.
     *  - maxIdleTime - maximum time that a worker thread will spend idle. If a
     *                  worker is idle longer than this time and there are more
     *                  than the min number of workers running, then the worker
     *                  thread exits.
     *  - jobsPerThread - ratio of jobs to thread to try and maintain if a job
     *                    is scheduled and the number of jobs per thread is
     *                    greater than this number,and if less than the maximum
     *                    number of workers are running then a new thread is
     *                    started to help out with efficiency.
     *  - schedPolicy - scheduling policy to try and set (OS dependent).
     */
    ThreadPoolAttr* attr);

/*!
 * \brief Adds a persistent job to the thread pool.
 *
 * Job will be run as soon as possible. Call will block until job is scheduled.
 *
 * \returns
 *  On success: **0**\n
 *  On error:
 *  - EOUTOFMEM not enough memory to add job.
 *  - EMAXTHREADS not enough threads to add persistent job.
 */
int ThreadPoolAddPersistent(
    /*! [in] Valid thread pool pointer. */
    ThreadPool* tp,
    /*! [in] Valid thread pool job. */
    ThreadPoolJob* job,
    /*! [in] Job ID */
    int* jobId);

/*!
 * \brief Gets the current set of attributes associated with the thread pool.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 */
int ThreadPoolGetAttr(
    /*! [in] Valid thread pool pointer. */
    ThreadPool* tp,
    /*! [in] Non null pointer to store attributes. */
    ThreadPoolAttr* out);

/*!
 * \brief Sets the attributes for the thread pool.
 *
 * Only affects future calculations.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 *  - INVALID_POLICY if policy can not be set.
 */
int ThreadPoolSetAttr(
    /*! [in] Valid thread pool pointer. */
    ThreadPool* tp,
    /*! [in] Pointer to attributes, null sets attributes to default. */
    ThreadPoolAttr* attr);

/*!
 * \brief Adds a job to the thread pool.
 *
 * Job will be run as soon as possible.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 *  - EOUTOFMEM if not enough memory to add job.
 */
int ThreadPoolAdd(
    /*! [in] Valid thread pool pointer. */
    ThreadPool* tp,
    /*! [in] Job */
    ThreadPoolJob* job,
    /*! [in] id of job. */
    int* jobId);

/*!
 * \brief Removes a job from the thread pool.
 *
 * Can only remove jobs which are not currently running.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 *  - INVALID_JOB_ID if job not found.
 */
int ThreadPoolRemove(
    /*! [in] Valid thread pool pointer. */
    ThreadPool* tp,
    /*! [in] Id of job. */
    int jobId,
    /*! [out] Space for removed job. */
    ThreadPoolJob* out);

/*!
 * \brief Shuts the thread pool down.
 *
 * Waits for all threads to finish. May block indefinitely if jobs do not exit.
 *
 * \returns
 *  On success: **0**\n
 *  On error: nonzero
 */
int ThreadPoolShutdown(
    /*! [in] Must be valid tp. */
    ThreadPool* tp);

/*!
 * \brief Initializes thread pool job.
 *
 * Sets the priority to default defined in ThreadPool.hpp. Sets the
 * free_routine to default defined in ThreadPool.hpp.
 *
 * \returns Always **0**.
 */
int TPJobInit(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolJob* job,
    /*! [in] Function to run, must be valid. */
    start_routine func,
    /*! [in] Argument to pass to function. */
    void* arg);

/*!
 * \brief Sets the priority of the threadpool job.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPJobSetPriority(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolJob* job,
    /*! [in] Value to set. */
    ThreadPriority priority);

/*!
 * \brief Sets the jobs free function.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPJobSetFreeFunction(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolJob* job,
    /*! [in] Value to set. */
    free_routine func);

/*!
 * \brief Initializes thread pool attributes.
 *
 * Sets values to defaults defined in ThreadPool.hpp.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrInit(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr);

/*!
 * \brief Sets the max threads for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrSetMaxThreads(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Value to set. */
    int maxThreads);

/*!
 * \brief Sets the min threads for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrSetMinThreads(
    /*! [in] must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] value to set. */
    int minThreads);

/*!
 * \brief Sets the stack size for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrSetStackSize(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Value to set. */
    size_t stackSize);

/*!
 * \brief Sets the idle time for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrSetIdleTime(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Idle time */
    int idleTime);

/*!
 * \brief Sets the jobs per thread ratio.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EINVAL.
 */
int TPAttrSetJobsPerThread(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Number of jobs per thread to maintain. */
    int jobsPerThread);

/*!
 * \brief Sets the starvation time for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On  error: EINVAL.
 */
int TPAttrSetStarvationTime(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Milliseconds. */
    int starvationTime);

/*!
 * \brief Sets the scheduling policy for the thread pool attributes.
 *
 * \returns
 *  On success: **0**\n
 *  On  error: EINVAL.
 */
int TPAttrSetSchedPolicy(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Must be a valid policy type. */
    PolicyType schedPolicy);

/*!
 * \brief Sets the maximum number jobs that can be qeued totally.
 *
 * \returns
 *  On success: **0**\n
 *  On  error: EINVAL.
 */
int TPAttrSetMaxJobsTotal(
    /*! [in] Must be valid thread pool attributes. */
    ThreadPoolAttr* attr,
    /*! [in] Maximum number of jobs. */
    int maxJobsTotal);

/*!
 * \brief Returns various statistics about the thread pool.
 *
 * Only valid if STATS has been defined.
 *
 * \returns
 *  On success: **0**\n
 *  On  error: EINVAL.
 */
#if defined(STATS) || defined(DOXYGEN_RUN)
int ThreadPoolGetStats(
    /*! [in] Valid initialized threadpool. */
    ThreadPool* tp,
    /*! [out] Valid stats, out parameter. */
    ThreadPoolStats* stats);
#else
static inline int ThreadPoolGetStats(
    /*! [in] Valid initialized threadpool. */
    ThreadPool* tp,
    /*! [out] Valid stats, out parameter. */
    ThreadPoolStats* stats) {}
#endif

/*!
 * \brief Prints various statistics about the thread pool to stderr.
 *
 * Only valid if STATS has been defined.
 */
#if defined(STATS) || defined(DOXYGEN_RUN)
void ThreadPoolPrintStats(
    /*! [in] Valid threadpool stats */
    ThreadPoolStats* stats);
#else
static inline void ThreadPoolPrintStats(
    /*! [in] Valid threadpool stats */
    ThreadPoolStats* stats) {}
#endif

#endif /* COMPA_THREADPOOL_HPP */
